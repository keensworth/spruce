
// ╔═══════════════════════════════════╗
// ║     Vertex                        ║
// ╚═══════════════════════════════════╝
struct VertexPosition {
    vec4 pos;
};

struct VertexAttributes {
    vec4 normal_u;  // [ normal.xyz  |  tex.u ]
    vec4 color_v;   // [  color.xyz  |  tex.v ]
};


// ╔═══════════════════════════════════╗
// ║     Material                      ║
// ╚═══════════════════════════════════╝
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