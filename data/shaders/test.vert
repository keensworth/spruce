#version 460

#define SPR_GLOBAL_BINDINGS 1
#include "common_bindings.glsl"


vec2 positionsHard[4] = vec2[](
    vec2(-1.0,  1.0),
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0)
);

void main()
{   
    gl_Position = vec4(positionsHard[gl_VertexIndex], 0.0, 1.0);
}