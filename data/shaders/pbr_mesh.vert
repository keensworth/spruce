#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

layout(location = 0) out vec4 pos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 color;
layout(location = 3) out vec2 texCoord;
layout(location = 4) out flat uint drawId;
layout(location = 5) out vec4 viewPos;

void main() {
    DrawData draw = draws[gl_InstanceIndex];
    Transform transform = transforms[draw.transformOffset];
    Scene scene = sceneData;

    VertexAttributes att = attributes[gl_VertexIndex + draw.vertexOffset];

    vec4 positionLocal = positions[gl_VertexIndex + draw.vertexOffset].pos;

    pos = transform.model * positionLocal;
    normal = normalize(mat3(transform.modelInvTranspose) * att.normal_u.xyz);
    color = att.color_v.rgb;
    texCoord = vec2(att.normal_u.w, att.color_v.w);
    drawId = gl_InstanceIndex;
    viewPos = scene.view * pos;

    gl_Position = scene.viewProj * pos;
}
