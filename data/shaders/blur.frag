#version 460

#define SPR_BLUR 1
#include "common_util.glsl"

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 texCoord;
layout(location = 1) in flat vec2 screenDim;
layout(location = 2) in flat vec2 direction;

layout(set = 2, binding = 0) uniform sampler2D inputTex;

void main(){
    vec2 texCoords = vec2(gl_FragCoord.xy / textureSize(inputTex, 0));
    FragColor = blur9(inputTex, texCoords, screenDim, direction);
}