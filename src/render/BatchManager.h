#pragma once

#include <vector>
#include "spruce_core.h"
#include "util/BatchNode.h"

namespace spr::gfx{

typedef enum : uint32 {
    SPR_BASE_COLOR         = 1,
    SPR_METALLIC_ROUGHNESS = 1<<1,
    SPR_NORMAL             = 1<<2,
    SPR_OCCLUSION          = 1<<3,
    SPR_EMISSIVE           = 1<<4,
    SPR_ALPHA              = 1<<5,
    SPR_DOUBLE_SIDED       = 1<<6,
    SPR_UNLIT              = 1<<10,
    SPR_WIREFRAME          = 1<<11,
    SPR_RECEIVES_SHADOWS   = 1<<12,
    SPR_CASTS_SHADOWS      = 1<<13,
    SPR_REFLECTIVE         = 1<<14
} SprMaterialFlags;

class BatchManager{
public:
    BatchManager(){
        m_batches = BatchNode();
    }
    ~BatchManager(){}

    void addDraw(DrawData draw, uint32 meshId, uint32 materialFlags);
    std::vector<Batch> getBatches(uint32 materialFlags);
    void prepareDraws(VulkanResourceManager* rm);
    void reset(VulkanResourceManager* rm);
    void destroy(VulkanResourceManager* rm);
    
private:
    BatchNode m_batches;
};
}