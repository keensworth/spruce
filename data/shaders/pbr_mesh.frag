#version 460

#define SPR_GLOBAL_BINDINGS
#define SPR_FRAME_BINDINGS
#include "common_bindings.glsl"
#include "common_constants.glsl"

#define SPR_NORMALS
#include "common_util.glsl"

#define SPR_SHADOW_CASCADE_MAPS 2
#define SPR_SHADOW_CASCADE_DATA 3
#include "common_shadow.glsl"

layout(set = 2, binding = 0) uniform sampler2D depthMap;
layout(set = 2, binding = 1) uniform sampler2D occlusionMap;
// binding 2 & 3 defined in common_shadow.glsl
layout(set = 2, binding = 4) uniform sampler2D scatteringMap;

struct Cluster {
    uint offset;
    uint count;
};
layout (std430, set = 3, binding = 0) buffer ClusterList {
    Cluster clusters[];
};
layout (std430, set = 3, binding = 1) buffer LightList {
    uint lightIndices[];
};
layout (std430, set = 3, binding = 2) buffer GlobalIndexCount {
    uint globalIndexCount;
};

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in flat uint drawId;
layout(location = 5) in vec4 viewPos;

layout(location = 0) out vec4 FragColor;

const uvec3 clusterCounts = {16, 8, 24};

vec3 gtaoMultiBounce(float visibility, vec3 color) {
	vec3 a = vec3( 2.0404 * color - 0.3324);
	vec3 b = vec3(-4.7951 * color + 0.6417);
	vec3 c = vec3( 2.7552 * color + 0.6903);

	float x = visibility;
	return vec3(max(vec3(x), ((a * x + b) * x + c) * x));
}

float calculateShadow(vec3 N, vec3 lightDir) {
	uint cascadeIndex = 0;
	for(uint i = 0; i < MAX_SHADOW_CASCADES - 1; ++i) {
		if(-viewPos.z > shadowData.cascadeSplit[0][i]) {	
			cascadeIndex = i + 1;
		}
	}

	vec4 shadowCoord = (biasMat * shadowData.cascadeViewProj[cascadeIndex]) * pos;

	float mixedDist = shadowData.cascadeSplit[0][cascadeIndex];
	if (cascadeIndex > 0){
		float nearDist = shadowData.cascadeSplit[0][cascadeIndex-1];
		float farDist = mixedDist;
		mixedDist = mix(nearDist, farDist, (-viewPos.z-nearDist)/(farDist - nearDist));
	}

	float bias = min(0.05 * max(1.0 - dot(normal, lightDir), 0.0), 0.005);
	bias *= 1.0 / (mixedDist * 0.5);

	float shadow = filterPCF(shadowCoord, cascadeIndex, bias);
	return shadow;
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float NdLV, float roughness) {
	float alpha   = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (NdLV * NdLV) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k) {
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float NdL, float NdV, float roughness) {
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(NdL, k) * gaSchlickG1(NdV, k);
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta) {
	return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float getPointAttenuation(Light light){
	float d = length(light.pos - pos.rgb);
	float i = light.intensity;
	float r = light.range;
	float s = min(d/r, 1.0);
	float x = 1 - s * s;
	return i*((x * x) / (1.0 + 1.0 * s));
}

Cluster getCluster(){
	float zFar = camera.far;
	float zNear = camera.near;

	float sliceCount = clusterCounts.z;

	vec2 tileSizePx = vec2(sceneData.screenDimX / clusterCounts.x, sceneData.screenDimY / clusterCounts.y);

	float z = -viewPos.z;

	uvec3 clusterPos;
	clusterPos.xy = uvec2(gl_FragCoord.xy / tileSizePx);
	clusterPos.z = uint((log(abs(z) / zNear) * sliceCount) / log(zFar / zNear));

	uint clusterIndex = clusterPos.x + 
						clusterPos.y * clusterCounts.x +
						clusterPos.z * clusterCounts.x * clusterCounts.y;

	return clusters[clusterIndex];
}

struct LightingParams {
	vec4 baseColor;
	float metalness;
	float roughness;
	vec3 emissive;
	vec3 V;
	vec3 N;
	float NdV;
	vec3 Lr;
	vec3 F0;
};

LightingParams getLightingParams(MaterialData material){
	vec4 baseColor = texture(textures[material.baseColorTexIdx], texCoord).rgba;
    baseColor *= material.baseColorFactor;

	if (baseColor.a < material.alphaCutoff){
		discard;
	}

    vec3 mapNormal = texture(textures[material.normalTexIdx], texCoord).rgb;
	mapNormal = normalize(mapNormal * 2.0 - 1.0);
    mapNormal *= vec3(material.normalScale, material.normalScale, 1.0);

    float mapMetal = texture(textures[material.metalRoughTexIdx], texCoord).b;
    mapMetal *= material.metallicFactor;
    mapMetal = clamp(mapMetal, 0.0, 1.0);

    float mapRoughness = texture(textures[material.metalRoughTexIdx], texCoord).g;
    mapRoughness *= material.roughnessFactor;
    mapRoughness = clamp(mapRoughness, 0.04, 1.0);

	vec3 mapEmissive = texture(textures[material.emissiveTexIdx], texCoord).rgb;
	mapEmissive *= material.emissiveFactor;

    // ws_frag -> ws_camera
	vec3 V = normalize(camera.pos - pos.rgb);

	// world normal, after applying normal map
	vec3 N = perturb_normal(normal, camera.pos - pos.xyz, texCoord, mapNormal);
	
	// Angle between surface normal and outgoing light direction.
	float NdV = max(0.0, dot(N, V));
		
	// Specular reflection vector.
	vec3 Lr = 2.0 * NdV * N - V;

	// Fresnel reflectance at normal incidence (for metals use albedo color).
	vec3 F0 = mix(vec3(0.04), baseColor.rgb, mapMetal);

	return LightingParams(baseColor, mapMetal, mapRoughness, mapEmissive, V, N, NdV, Lr, F0);
}

vec3 calculateDirectLighting(LightingParams p, Light light, vec3 L, float attenuation, float shadow) {
	// radiance of current light
	vec3 Lr = light.color * attenuation * light.intensity;

	// Half-vector between L and V.
	vec3 LhV = normalize(L + p.V);

	// Calculate angles between surface normal and various light vectors.
	float NdL = max(0.0, dot(p.N, L));
	float NdLV = max(0.0, dot(p.N, LhV));

	// Calculate Fresnel term for direct lighting. 
	vec3 F  = fresnelSchlick(p.F0, max(0.0, dot(LhV, p.V)));
	// Calculate normal distribution for specular BRDF.
	float D = ndfGGX(NdLV, p.roughness);
	// Calculate geometric attenuation for specular BRDF.
	float G = gaSchlickGGX(NdL, p.NdV, p.roughness);

	// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
	// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
	// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
	vec3 kD = mix(vec3(1.0) - F, vec3(0.0), p.metalness);

	// Lambert diffuse BRDF.
	vec3 diffuseBRDF = kD * p.baseColor.rgb;

	// Cook-Torrance specular microfacet BRDF.
	vec3 specularBRDF = (F * D * G) / max(0.00001, 4.0 * NdL * p.NdV);

	// Total contribution for this light.
	return (diffuseBRDF + specularBRDF) * Lr * NdL * shadow;
} 

vec3 calculateAmbientLighting(LightingParams p) {
	vec2 uv = vec2(gl_FragCoord.xy / textureSize(occlusionMap, 0));
	vec3 visibility = texture(occlusionMap, uv).rgb;
	visibility = gtaoMultiBounce(visibility.x, p.baseColor.rgb);

	vec3 F = fresnelSchlick(p.F0, p.NdV);
	vec3 kd = mix(vec3(1.0) - F, vec3(0.0), p.metalness);

	vec3 diffuseIBL = kd * p.baseColor.rgb;
	
	vec3 ambientLighting;
	ambientLighting = diffuseIBL;
	ambientLighting *= visibility;
	ambientLighting += p.emissive;

	return ambientLighting;
}

void main() {
    DrawData draw = draws[drawId];
    MaterialData material = materials[draw.materialOffset];

	LightingParams params = getLightingParams(material);

	// get relevant light cluster
	Cluster cluster = getCluster();
	uint offset = cluster.offset;
	uint count = cluster.count;

	// direct lighting contributions from point lights in cluster
	vec3 directLighting = vec3(0.0);
	for (int i = 0; i < count; i++){
		Light light = lights[lightIndices[offset + i]];
		vec3 L = normalize(light.pos - pos.rgb);
		float attenuation = getPointAttenuation(light);
		directLighting += calculateDirectLighting(params, light, L, attenuation, 1.0);
	}

	{
		Light light = lights[sceneData.sunOffset];
		vec3 L = -light.dir;
		float attenuation = 1.0;
		float shadow = calculateShadow(params.N, light.dir);
		directLighting += calculateDirectLighting(params, light, L, attenuation, shadow);
	}

	// ambient ligthing
	vec3 ambientLighting = calculateAmbientLighting(params);

	// vec2 texCoords = vec2(gl_FragCoord.xy / textureSize(depthMap, 0));
	// vec4 scatteringInfo = texture(scatteringMap, texCoords).rgba;
	// vec3 inScattering = scatteringInfo.rgb;
	// float transmittance = scatteringInfo.a;
	vec4 pixelColorWithoutFog = vec4(directLighting + ambientLighting, 1.0);
    //FragColor = pixelColorWithoutFog * vec4(transmittance.xxx, 1.0) + vec4(inScattering, 0.0);
    FragColor = pixelColorWithoutFog;
}