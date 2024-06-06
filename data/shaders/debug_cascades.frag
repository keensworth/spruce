#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"
#include "common_constants.glsl"

#define SPR_SHADOW_CASCADE_DATA 0
#include "common_shadow.glsl"

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in flat uint drawId;
layout(location = 5) in vec4 viewPos;

layout(location = 0) out vec4 FragColor;


void main() {
    DrawData draw = draws[drawId];
    MaterialData material = materials[draw.materialOffset];
    Scene scene = sceneData;

    vec4 color = vec4(texture(textures[material.baseColorTexIdx], texCoord).rgb, 0.7);

    uint cascadeIndex = 0;
	for(uint i = 0; i < MAX_SHADOW_CASCADES - 1; ++i) {
		if(-viewPos.z > shadowData.cascadeSplit[0][i]) {	
			cascadeIndex = i + 1;
		}
	}

    vec4 cascadeColor = vec4(0.0);
    if (cascadeIndex == 0){
        cascadeColor = vec4(1.0, 0.0, 0.0, 0.3);
    } else if (cascadeIndex == 1){
        cascadeColor = vec4(0.0, 1.0, 0.0, 0.3);
    } else if (cascadeIndex == 2){
        cascadeColor = vec4(0.0, 0.0, 1.0, 0.3);
    } else if (cascadeIndex == 3){
        cascadeColor = vec4(1.0, 1.0, 0.0, 0.3);
    } else {
        cascadeColor = vec4(1.0, 1.0, 1.0, 0.3);
    }

    FragColor = color + cascadeColor;
}