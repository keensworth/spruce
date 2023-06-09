#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

layout(location = 0) in vec2 texCoord;
layout(location = 1) in flat uint drawId;

layout(location = 0) out vec4 FragColor;

void main() {
    DrawData draw = draws[drawId];
    MaterialData material = materials[draw.materialOffset];
    vec4 color = vec4(texture(textures[material.baseColorTexIdx], texCoord).rgb, 1.0);
    FragColor = color;
}