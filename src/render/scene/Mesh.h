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
    glm::vec4 normal3_u1; // [  normal.xyz  | tex.u ]
    glm::vec4 color3_v1;  // [  color.xyz   | tex.v ]
} VertexAttributes;

}