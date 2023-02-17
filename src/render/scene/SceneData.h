#pragma once

#include "spruce_core.h"

namespace spr::gfx {

typedef struct Transform {
    glm::mat4 model;
    glm::mat4 modelInvTranspose;
} Transform;

typedef struct MeshData {
    uint32 vertexOffset;
    uint32 indexCount;
    uint32 materialIndex;
    uint32 padding;
} MeshData;

typedef enum LightTypes {
    POINT,
    SPOT,
    DIRECTIONAL
} LightTypes;

typedef struct SpotProperties {
    float innerAngle;
    float outerAngle;
} SpotProperties;

typedef struct Light {
    glm::vec3 pos;
    uint32 type;
    glm::vec3 dir;
    float intensity;
    glm::vec3 color;
    float range;
} Light;

typedef struct Camera {
    glm::vec3 pos;
    float fov;
    glm::vec3 dir;
    float near;
    glm::vec3 up;
    float far;
} Camera;

typedef struct Scene {
    glm::mat4x4 view;
    glm::mat4x4 proj;
    Camera camera;
} Scene;

}