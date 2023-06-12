#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

layout(location = 0) out vec2 texCoord;
layout(location = 1) out flat uint drawId;

void main() {
    DrawData draw = draws[gl_InstanceIndex];
    Transform transform = transforms[draw.transformOffset];
    Scene scene = sceneData;

    VertexAttributes att = attributes[gl_VertexIndex + draw.vertexOffset];
    texCoord = vec2(att.normal_u.w, att.color_v.w);
    drawId = gl_InstanceIndex;

    vec4 positionLocal = positions[gl_VertexIndex + draw.vertexOffset].pos;
    vec4 pos = transform.model * positionLocal;

    gl_Position = scene.viewProj * pos;
}
