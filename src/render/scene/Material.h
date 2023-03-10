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

}