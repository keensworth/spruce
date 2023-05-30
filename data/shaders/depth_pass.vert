#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

void main()
{
    DrawData draw = draws[gl_InstanceIndex];
    Transform transform = transforms[draw.transformOffset];
    Scene scene = sceneData;

    VertexAttributes att = attributes[gl_VertexIndex + draw.vertexOffset];

    vec4 positionLocal = positions[gl_VertexIndex + draw.vertexOffset].pos;
    vec4 pos = transform.model * positionLocal;

    gl_Position = scene.viewProj * pos;
}
