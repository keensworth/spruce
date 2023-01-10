#pragma once

#include <vector>
#include "spruce_core.h"
#include "BatchNode.h"

namespace spr::gfx{

typedef enum : uint32 {
    BASE_COLOR         = 1,
    METALLIC_ROUGHNESS = 1<<1,
    NORMAL             = 1<<2,
    OCCLUSION          = 1<<3,
    EMISSIVE           = 1<<4,
    ALPHA              = 1<<5,
    DOUBLE_SIDED       = 1<<6,
    UNLIT              = 1<<10,
    WIREFRAME          = 1<<11,
    RECEIVES_SHADOWS   = 1<<12,
    CASTS_SHADOWS      = 1<<13,
    REFLECTIVE         = 1<<14
} MaterialFlags;

class BatchManager{
public:
    BatchManager(){
        m_batches = BatchNode();
    }
    ~BatchManager(){}

    void addDraw(DrawData draw, uint32 meshId, uint32 materialFlags);
    std::vector<Batch> getBatches(uint32 materialFlags);
    void prepareDraws();
    void reset();
    void destroy();
    
private:
    BatchNode m_batches;
};
}