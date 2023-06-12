
// ╔═══════════════════════════════════╗
// ║     Cascade Types/Constants       ║
// ╚═══════════════════════════════════╝

const uint MAX_SHADOW_CASCADES = 4;

struct SunShadowData {	
	mat4 cascadeViewProj[MAX_SHADOW_CASCADES];
	mat4 cascadeSplit;
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, -0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 
);


// ╔═══════════════════════════════════╗
// ║     Cascade Data Bindings         ║
// ╚═══════════════════════════════════╝

#ifndef SPR_SHADOW_CASCADE_DATA
#define SPR_SHADOW_CASCADE_DATA -1
#endif

#if SPR_SHADOW_CASCADE_DATA > -1
layout(set = 2, binding = SPR_SHADOW_CASCADE_DATA) uniform CascadeData {
    SunShadowData shadowData;
};
#endif


// ╔═══════════════════════════════════╗
// ║     Cascade Map Bindings + Util   ║
// ╚═══════════════════════════════════╝

#ifndef SPR_SHADOW_CASCADE_MAPS
#define SPR_SHADOW_CASCADE_MAPS -1
#endif

#if SPR_SHADOW_CASCADE_MAPS > -1
layout(set = 2, binding = SPR_SHADOW_CASCADE_MAPS) uniform sampler2D sunShadowMaps[MAX_SHADOW_CASCADES];

float linearizeDepth(float d,float zNear,float zFar){
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

float inverseDepth(float sampledDepth){
	float d = 1.0 - sampledDepth;
	return linearizeDepth(d, camera.near, camera.far);
}

float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex, float bias) {
	float shadow = 1.0;

	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) {
		float dist = texture(sunShadowMaps[cascadeIndex], vec2(shadowCoord.st + offset)).r;
		if (shadowCoord.w > 0 && dist > shadowCoord.z + bias) {
			shadow = 0.0;
		}
	}
	return shadow;
}

float filterPCF(vec4 shadowCoord, uint cascadeIndex, float bias) {
	ivec2 texDim = textureSize(sunShadowMaps[cascadeIndex], 0).xy;
	float scale = 0.25;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++) {
		for (int y = -range; y <= range; y++) {
			shadowFactor += textureProj(shadowCoord, vec2(dx*x, dy*y), cascadeIndex, bias);
			count++;
		}
	}
	return shadowFactor / count;
}
#endif


