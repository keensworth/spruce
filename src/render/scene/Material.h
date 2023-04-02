#pragma once

#include "spruce_core.h"


namespace spr::gfx { 

typedef enum : uint32 {
    MTL_BASE_COLOR         = 1,
    MTL_METALLIC_ROUGHNESS = 1<<1,
    MTL_NORMAL             = 1<<2,
    MTL_OCCLUSION          = 1<<3,
    MTL_EMISSIVE           = 1<<4,
    MTL_ALPHA              = 1<<5,
    MTL_DOUBLE_SIDED       = 1<<6,
    MTL_UNLIT              = 1<<10,
    MTL_WIREFRAME          = 1<<11,
    MTL_RECEIVES_SHADOWS   = 1<<12,
    MTL_CASTS_SHADOWS      = 1<<13,
    MTL_REFLECTIVE         = 1<<14,
    MTL_ALL                = 0xFFFFFFFF,
    MTL_NONE               = 0x00000000
} MaterialFlags;

typedef struct MaterialData {
    uint32 baseColorTexIdx    = 0; // default_color
    uint32 metalRoughTexIdx   = 1; // default_input_black
    uint32 normalTexIdx       = 1; // default_input_black
    uint32 occlusionTexIdx    = 1; // default_input_black

    uint32 emissiveTexIdx     = 1; // default_input_black
    float metallicFactor      = 1;
    float roughnessFactor     = 1;
    float normalScale         = 1;

    glm::vec4 baseColorFactor = {1.f,1.f,1.f,1.f};

    glm::vec3 emissiveFactor  = {0.f,0.f,0.f};
    float occlusionStrength   = 1;

    uint32 alpha      = 0;
    float alphaCutoff = 0.5f;
    uint32 flags      = MTL_NONE;
    uint32 pad;
} MaterialData;

}