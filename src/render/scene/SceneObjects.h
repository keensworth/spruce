#pragma once

#include "spruce_core.h"

namespace spr::gfx {

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

}