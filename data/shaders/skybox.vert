#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

layout(location = 0) out vec3 texCoord;

void main() {
    Scene scene = sceneData;

    VertexAttributes att = attributes[gl_VertexIndex];

    vec4 positionLocal = positions[gl_VertexIndex].pos;
    vec3 pos = positionLocal.xyz;

    texCoord = positionLocal.xyz;
    texCoord.x *= -1.0;
    texCoord.yz = texCoord.zy;

    pos += camera.pos;
    gl_Position = scene.viewProj * vec4(pos, 1.0);
}
