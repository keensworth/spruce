#version 450

struct VertexPosition {
    vec3 pos;
};

struct VertexAttributes {
    vec4 normal_u;
    vec4 color_v;
};
 
struct MaterialData {
    uint baseColorTexIdx;
    uint metalRoughTexIdx;
    uint normalTexIdx;
    uint occlusionTexIdx;

    uint emissiveTexIdx;
    float metallicFactor;
    float roughnessFactor;
    float normalScale;

    vec4 baseColorFactor;

    vec3 emissiveFactor;
    float occlusionStrength;

    uint alpha;
    float alphaCutoff;
    uint flags;
    uint pad;
};

layout(set = 0, binding = 0) readonly buffer Positions {
    VertexPosition positions[];
};

layout(set = 0, binding = 1) readonly buffer Attributes {
    VertexAttributes attributes[];
};

layout(std140,set = 0, binding = 2) readonly buffer Materials {
    MaterialData materials[];
};

layout(set = 0, binding = 3) uniform texture2D textures[1024];



vec2 positionsHard[4] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0,  1.0),
    vec2(-1.0,  1.0)
);


void main()
{   
    gl_Position = vec4(positionsHard[gl_VertexIndex], 0.0, 1.0);
    //gl_Position = vec4(positions[gl_VertexIndex].pos, 1.0);
}