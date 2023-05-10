#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"


void main()
{
    DrawData draw = draws[gl_InstanceIndex];
    Transform transform = transforms[draw.transformOffset];
    Scene scene = sceneData;

    vec4 positionLocal = positions[gl_VertexIndex + draw.vertexOffset].pos;
    
    gl_Position = scene.viewProj * transform.model * positionLocal;
}
