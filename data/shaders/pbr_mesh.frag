#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"
#include "common_constants.glsl"

#define SPR_NORMALS 1
#include "common_util.glsl"

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in flat uint drawId;


layout(set = 2, binding = 0) uniform sampler2D depthMap;
layout(set = 2, binding = 1) uniform sampler2D occlusionMap;


vec3 gtaoMultiBounce(float visibility, vec3 color){
	vec3 a = vec3( 2.0404 * color - 0.3324);
	vec3 b = vec3(-4.7951 * color + 0.6417);
	vec3 c = vec3( 2.7552 * color + 0.6903);

	float x = visibility;
	return vec3(max(vec3(x), ((a * x + b) * x + c) * x));
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void main(){
    DrawData draw = draws[drawId];
    MaterialData material = materials[draw.materialOffset];
    uint lightCount = sceneData.lightCount;

    vec4 baseColor = vec4(texture(textures[material.baseColorTexIdx], texCoord).rgb, 1.0);
    baseColor *= material.baseColorFactor;// * vec4(color,1.0);
    vec3 mapNormal = texture(textures[material.normalTexIdx], texCoord).rgb;
	mapNormal = normalize(mapNormal * 2.0 - 1.0);
    mapNormal *= vec3(material.normalScale, material.normalScale, 1.0);
    float mapMetal = texture(textures[material.metalRoughTexIdx], texCoord).b;
    mapMetal *= material.metallicFactor;
    mapMetal = clamp(mapMetal, 0.0, 1.0);
    float mapRoughness = texture(textures[material.metalRoughTexIdx], texCoord).g;
    mapRoughness *= material.roughnessFactor;
    mapRoughness = clamp(mapRoughness, 0.04, 1.0);
    

    // Outgoing light direction (vector from world-space fragment position to the "eye").
	vec3 Lo = normalize(camera.pos - pos.rgb);

	// Get current fragment's normal and transform to world space.
	vec3 N = perturb_normal(normal, camera.pos - pos.xyz, texCoord, mapNormal);
	
	// Angle between surface normal and outgoing light direction.
	float cosLo = max(0.0, dot(N, Lo));
		
	// Specular reflection vector.
	vec3 Lr = 2.0 * cosLo * N - Lo;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	vec3 F0 = mix(vec3(0.04), baseColor.rgb, mapMetal);

    vec3 directLighting = vec3(0.0);
    for(int i = 0; i < lightCount; ++i){
		vec3 Li;
		float attenuation;
		if (lights[i].type == DIRECTIONAL){
        	Li = -lights[i].dir;
			attenuation = 1.0;
		} else {
			Li = normalize(lights[i].pos - pos.rgb);
			float d = length(lights[i].pos - pos.rgb);
			//attenuation = (clamp(lights[i].range - d, 0.0, lights[i].range)) / (1.0 + 0.5*d*d);
			attenuation = 1.0 / (1.0 + 0.5*d*d);
		}
        vec3 Lradiance = lights[i].color * attenuation * lights[i].intensity;

		// Half-vector between Li and Lo.
		vec3 Lh = normalize(Li + Lo);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(N, Li));
		float cosLh = max(0.0, dot(N, Lh));

		// Calculate Fresnel term for direct lighting. 
		vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
		// Calculate normal distribution for specular BRDF.
		float D = ndfGGX(cosLh, mapRoughness);
		// Calculate geometric attenuation for specular BRDF.
		float G = gaSchlickGGX(cosLi, cosLo, mapRoughness);

		// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
		// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
		// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
		vec3 kd = mix(vec3(1.0) - F, vec3(0.0), mapMetal);

		// Lambert diffuse BRDF.
		vec3 diffuseBRDF = kd * baseColor.rgb;

		// Cook-Torrance specular microfacet BRDF.
		vec3 specularBRDF = (F * D * G) / max(0.00001, 4.0 * cosLi * cosLo);

		// Total contribution for this light.
		directLighting += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
    }

	vec3 ambientLighting;
	{
		vec3 mapEmissive = texture(textures[material.emissiveTexIdx], texCoord).rgb;
		mapEmissive *= material.emissiveFactor;
		
		vec2 uv = vec2(gl_FragCoord.xy / textureSize(occlusionMap, 0));
		vec3 visibility = texture(occlusionMap, uv).rgb;
		visibility = gtaoMultiBounce(visibility.x, baseColor.rgb);
		
		// vec3 irradiance = vec3(1.0, 0.9098, 0.6706) * 0.3;

		// vec3 F = fresnelSchlick(F0, cosLo);
		// vec3 kd = mix(vec3(1.0) - F, vec3(0.0), mapMetal);

		// vec3 diffuseIBL = kd * baseColor.rgb * irradiance;
		// vec3 specularIBL = vec3(0.0,0.0,0.0);

		ambientLighting = vec3(0.8) * baseColor.rgb;
		ambientLighting *= visibility;
		ambientLighting += mapEmissive;
	}
    
    FragColor = vec4(directLighting + ambientLighting, 1.0);
}