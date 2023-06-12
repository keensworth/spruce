
#include "common_scene_types.glsl"

// ╔══════════════════════════════════════════════════════════════════════════╗
// ║     Global (persist over frames)                                         ║
// ╚══════════════════════════════════════════════════════════════════════════╝

#ifdef SPR_GLOBAL_BINDINGS
layout(set = 0, binding = 0) readonly buffer Positions {
    VertexPosition positions[];
};

layout(std430,set = 0, binding = 1) readonly buffer Attributes {
    VertexAttributes attributes[];
};

layout(std430,set = 0, binding = 2) readonly buffer Materials {
    MaterialData materials[];
};

layout(set = 0, binding = 3) uniform sampler2D textures[1024];
#endif // SPR_GLOBAL_BINDINGS


// ╔══════════════════════════════════════════════════════════════════════════╗
// ║     Per-frame (bindless scene and per-draw data)                         ║
// ╚══════════════════════════════════════════════════════════════════════════╝

#ifdef SPR_FRAME_BINDINGS
layout(set = 1, binding = 0) uniform SceneData {
    Scene sceneData;
};

layout(set = 1, binding = 1) uniform CameraData {
    Camera camera;
};

layout(std430, set = 1, binding = 2) readonly buffer Lights {
    Light lights[];
};

layout(set = 1, binding = 3) readonly buffer Transforms {
    Transform transforms[];
};

layout(set = 1, binding = 4) readonly buffer Draws {
    DrawData draws[];
};
#endif // SPR_FRAME_BINDINGS