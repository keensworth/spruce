#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"


void main()
{
    DrawData draw = draws[gl_InstanceIndex];
    Transform transform = transforms[draw.transformOffset];
    Scene scene = sceneData;
    mat4 mvp = scene.viewProj * transform.model;
    
    vec4 positionLocal = positions[gl_VertexIndex + draw.vertexOffset].pos;
    //vec4 positionWorld = transform.model * positionLocal;
    gl_Position = mvp * positionLocal;
}
