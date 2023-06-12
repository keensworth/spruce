#version 460

#define SPR_GLOBAL_BINDINGS 1
#define SPR_FRAME_BINDINGS 1
#include "common_bindings.glsl"

#define SPR_SHADOW_CASCADE_DATA 0
#include "common_shadow.glsl"

layout(location = 0) out vec2 texCoord;
layout(location = 1) out flat uint drawId;

void main() {
    DrawData draw = draws[gl_InstanceIndex];
    Transform transform = transforms[draw.transformOffset];

    VertexAttributes att = attributes[(gl_VertexIndex-gl_BaseVertex) + draw.vertexOffset];
    texCoord = vec2(att.normal_u.w, att.color_v.w);
    drawId = gl_InstanceIndex;

    vec4 positionLocal = positions[(gl_VertexIndex-gl_BaseVertex) + draw.vertexOffset].pos;
    vec4 pos = transform.model * positionLocal;

    // use gl_BaseVertex to index into our light's viewProj array
    // [0, MAX_CASCADES), where lower values are closer to scene
    mat4 lightViewProj = shadowData.cascadeViewProj[gl_BaseVertex];
    gl_Position = lightViewProj * pos;
}
