#pragma once

#include <vector>
#include "spruce_core.h"


namespace spr::gfx {

typedef struct {
    uint32 vertexOffset;
    uint32 materialIndex;
    uint32 transformIndex;
    uint32 padding;
} DrawData;

typedef struct {
    uint32 meshId;
    uint32 materialFlags;
    uint32 indexCount;
    uint32 firstIndex;
    uint32 drawDataOffset;
    uint32 drawCount;
} Batch;

typedef struct {
    Batch batch;
    std::vector<DrawData> draws;
} DrawBatch;

typedef struct BatchMaterialQuery {
    uint32 hasAll     = 0;
    uint32 hasAny     = 0;
    uint32 hasExactly = 0;
    uint32 excludes   = 0;
} BatchMaterialQuery;

}