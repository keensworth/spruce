#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

#define SPR_SHADOW_CASCADE_DATA 0
#include "common_shadow.glsl"

layout(location = 0) in vec2 texCoord;
layout(location = 1) in flat uint drawId;

void main() {
    DrawData draw = draws[drawId];
    MaterialData material = materials[draw.materialOffset];

    vec4 baseColor = vec4(texture(textures[material.baseColorTexIdx], texCoord).rgba);
    baseColor *= material.baseColorFactor;// * vec4(color,1.0);
	if (baseColor.a < material.alphaCutoff){
		discard;
	}
}