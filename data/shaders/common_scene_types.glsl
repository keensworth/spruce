
// ╔═══════════════════════════════════╗
// ║     Vertex                        ║
// ╚═══════════════════════════════════╝
struct VertexPosition {
    vec4 pos;
};

struct VertexAttributes {
    vec4 normal_u;  // [  normal.xyz   |  tex.u ]
    vec4 color_v;   // [   color.xyz   |  tex.v ]
    vec4 tangent;   // [ tangent.xyzw  |        ]
};


// ╔═══════════════════════════════════╗
// ║     Material                      ║
// ╚═══════════════════════════════════╝

const uint MTL_BASE_COLOR         = 1;
const uint MTL_METALLIC_ROUGHNESS = 1<<1;
const uint MTL_NORMAL             = 1<<2;
const uint MTL_OCCLUSION          = 1<<3;
const uint MTL_EMISSIVE           = 1<<4;
const uint MTL_ALPHA              = 1<<5;
const uint MTL_DOUBLE_SIDED       = 1<<6;
const uint MTL_UNLIT              = 1<<10;
const uint MTL_WIREFRAME          = 1<<11;
const uint MTL_RECEIVES_SHADOWS   = 1<<12;
const uint MTL_CASTS_SHADOWS      = 1<<13;
const uint MTL_REFLECTIVE         = 1<<14;
const uint MTL_ALL                = 0xFFFFFFFF;
const uint MTL_NONE               = 0x00000000;

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


// ╔═══════════════════════════════════╗
// ║     Transform                     ║
// ╚═══════════════════════════════════╝
struct Transform {
    mat4 model;
    mat4 modelInvTranspose;
};


// ╔═══════════════════════════════════╗
// ║     Draw Data                     ║
// ╚═══════════════════════════════════╝
struct DrawData {
    uint vertexOffset;
    uint materialOffset;
    uint transformOffset;
    uint padding; // unused
};


// ╔═══════════════════════════════════╗
// ║     Light                         ║
// ╚═══════════════════════════════════╝
struct SpotProperties {
    float innerAngle;
    float outerAngle;
};

const uint POINT        = 0x00000000u;
const uint SPOT         = 0x00000001u;
const uint DIRECTIONAL  = 0x00000002u;

struct Light {
    vec3 pos;
    float intensity;
    vec3 dir;
    float range;
    vec3 color;
    uint type; // 0-point, 1-spot, 2-directional
    SpotProperties spotProps;
    uint pad1;
    uint pad2;
};


// ╔═══════════════════════════════════╗
// ║     Camera/Scene                  ║
// ╚═══════════════════════════════════╝
struct Camera {
    vec3 pos;
    float fov;
    vec3 dir;
    float near;
    vec3 up;
    float far;
};

struct Scene {
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    uint lightCount;
    uint sunOffset;
    uint screenDimX;
    uint screenDimY;
    uint time;
    float exposure;
};