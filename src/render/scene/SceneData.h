#pragma once

#include "spruce_core.h"
#include <numbers>

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

typedef enum LightType {
    POINT,
    SPOT,
    DIRECTIONAL
} LightType;

typedef struct SpotProperties {
    float innerAngle = 3.1415f/4.f;
    float outerAngle = 3.1415f/3.f;
} SpotProperties;

typedef struct Light {
    glm::vec3 pos   = {0.f, 0.f, 0.f};
    float intensity = 1.f;
    glm::vec3 dir   = {0.f, 1.f, 0.f};
    float range     = 16.f;
    glm::vec3 color = {1.f, 1.f, 1.f};
    SpotProperties spotProps;
    LightType type  = POINT;
} Light;

typedef struct Camera {
    glm::vec3 pos = {0.f, 0.f, 0.f};
    float fov     = 3.1415f/3.f;
    glm::vec3 dir = {0.f, 1.f, 0.f};
    float near    = 0.01f;
    glm::vec3 up  = {0.f, 0.f, 1.f};
    float far     = 1024.f;
} Camera;

typedef struct Scene {
    glm::mat4x4 view;
    glm::mat4x4 proj;
} Scene;

}