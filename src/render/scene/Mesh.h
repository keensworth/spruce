#pragma once

#include "spruce_core.h"

namespace spr::gfx {

typedef struct MeshInfo {
    uint32 vertexOffset;
    uint32 indexCount;
    uint32 firstIndex;
    uint32 materialIndex;
} MeshInfo;

typedef struct VertexPosition {
    glm::vec4 vertexPos;
} VertexPosition;

typedef struct VertexAttributes {
    glm::vec4 normal_u;
    glm::vec4 color_v;
} VertexAttributes;

}