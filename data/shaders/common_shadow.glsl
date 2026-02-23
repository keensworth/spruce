
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
// reverse-Z: compare op is LESS_OR_EQUAL — hardware returns 1.0 (lit) when stored_depth <= reference
layout(set = 2, binding = SPR_SHADOW_CASCADE_MAPS) uniform sampler2DShadow sunShadowMaps[MAX_SHADOW_CASCADES];

float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex, float bias) {
	return texture(sunShadowMaps[cascadeIndex], vec3(shadowCoord.st + offset, shadowCoord.z + bias));
}

float filterPCF(vec4 shadowCoord, uint cascadeIndex, float bias) {
	ivec2 texDim = textureSize(sunShadowMaps[cascadeIndex], 0).xy;
	float scale = 0.75;
	float dx = scale / float(texDim.x);
	float dy = scale / float(texDim.y);

	float shadowFactor = 0.0;
	int range = 1;

	for (int x = -range; x <= range; x++) {
		for (int y = -range; y <= range; y++) {
			shadowFactor += textureProj(shadowCoord, vec2(dx*x, dy*y), cascadeIndex, bias);
		}
	}
	return shadowFactor / 9.0;
}
#endif


