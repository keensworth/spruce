#version 460

#define SPR_GLOBAL_BINDINGS 1
#include "common_bindings.glsl"

layout(location = 0) out vec2 texCoord;

vec2 positionsHard[4] = vec2[](
    vec2(-1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0)
);

void main() {   
    vec4 pos = vec4(positionsHard[gl_VertexIndex], 0.0, 1.0);

    float x = pos.x;
    float y = -pos.y;
    texCoord.x = x * 0.5 + 0.5;
    texCoord.y = y * 0.5 + 0.5;
    
    gl_Position = pos;
}