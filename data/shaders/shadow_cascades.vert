#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

layout(set = 2, binding = 0) uniform CascadeData {
    mat4[4] cascadeViewProj;
};

void main() {
    DrawData draw = draws[gl_InstanceIndex];
    Transform transform = transforms[draw.transformOffset];

    vec4 positionLocal = positions[gl_VertexIndex + draw.vertexOffset].pos;
    vec4 pos = transform.model * positionLocal;

    // use gl_BaseInstance to index into our frustum viewProj array
    gl_Position = cascadeViewProj[gl_BaseInstance] * pos;
}
