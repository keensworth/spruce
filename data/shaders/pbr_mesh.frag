#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"
#include "common_constants.glsl"

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in flat uint drawId;

vec3 worldNormal(vec3 geomNormal, vec3 mapNormal){
    mapNormal = mapNormal * 2.0 - 1.0;
    mapNormal.x = -mapNormal.x;
    vec3 up = normalize(vec3(0.0001, 1, 0.0001));
    vec3 surfaceTangent = normalize(cross(geomNormal, up));
    vec3 surfaceBinormal = normalize(cross(geomNormal, surfaceTangent));
    return normalize(mapNormal.y * surfaceTangent + mapNormal.x * surfaceBinormal + mapNormal.z * geomNormal);
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
    mapNormal *= vec3(material.normalScale, material.normalScale, 1.0);
    float mapMetal = texture(textures[material.metalRoughTexIdx], texCoord).b;
    mapMetal *= material.metallicFactor;
    mapMetal = clamp(mapMetal, 0.0, 1.0);
    float mapRoughness = texture(textures[material.metalRoughTexIdx], texCoord).g;
    mapRoughness *= material.roughnessFactor;
    mapRoughness = clamp(mapRoughness, 0.04, 1.0);
    vec3 mapEmissive = texture(textures[material.emissiveTexIdx], texCoord).rgb;
	mapEmissive *= material.emissiveFactor;
	// occlusion
    

    // Outgoing light direction (vector from world-space fragment position to the "eye").
	vec3 Lo = normalize(camera.pos - pos.rgb);

	// Get current fragment's normal and transform to world space.
	vec3 N = worldNormal(normal, mapNormal);
	
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
			attenuation = (clamp(lights[i].range - d, 0.0, lights[i].range)) / (1.0 + 0.5*d*d);
		}
        float d = length(lights[i].pos - pos.rgb);
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
		// // Sample diffuse irradiance at normal direction.
		// vec3 irradiance = texture(irradianceTexture, N).rgb;

		// // Calculate Fresnel term for ambient lighting.
		// // Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
		// // use cosLo instead of angle with light's half-vector (cosLh above).
		// // See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
		// vec3 F = fresnelSchlick(F0, cosLo);

		// // Get diffuse contribution factor (as with direct lighting).
		// vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

		// // Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
		// vec3 diffuseIBL = kd * albedo * irradiance;

		// // Sample pre-filtered specular reflection environment at correct mipmap level.
		// int specularTextureLevels = textureQueryLevels(specularTexture);
		// vec3 specularIrradiance = textureLod(specularTexture, Lr, roughness * specularTextureLevels).rgb;

		// // Split-sum approximation factors for Cook-Torrance specular BRDF.
		// vec2 specularBRDF = texture(specularBRDF_LUT, vec2(cosLo, roughness)).rg;

		// // Total specular IBL contribution.
		// vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

		vec3 diffuseIBL = vec3(0.0,0.0,0.0);
		vec3 specularIBL = vec3(0.0,0.0,0.0);

		// Total ambient lighting contribution.
		ambientLighting = diffuseIBL + specularIBL + mapEmissive;
	}
    
    FragColor = vec4(directLighting + ambientLighting, 1.0);
}