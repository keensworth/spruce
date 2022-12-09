#pragma once

#include <vector>
#include "spruce_core.h"
#include "../../../external/flat_hash_map/flat_hash_map.hpp"
#include "../vulkan/resource/VulkanResourceManager.h"

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
    uint32 drawDataOffset;
    uint32 drawCount;
} Batch;

typedef struct {
    Batch batch;
    std::vector<DrawData> draws;
} BatchDraws;

class BatchNode {
public:
    BatchNode();
    BatchNode(uint32 height);
    ~BatchNode() {
        for (uint32 i = 0; i < m_nodeData.size(); i++){
            delete m_nodeData.at(i);
        }
    }

    //add
    void add(uint32 materialFlags, uint32 meshId, DrawData draw);
    //remove
    void remove(uint32 materialFlags);
    //get
    std::vector<Batch> get(uint32 materialFlags);
    //get_accum
    std::vector<Batch> getAny(uint32 materialFlags);

    void uploadDrawData(VulkanResourceManager* rm);

    uint32 getHeight(){
        return m_height;
    }

private:
    uint32 m_height;
    uint32 m_mask;
    std::vector<BatchNode*> m_nodeData;
    std::vector<ska::flat_hash_map<uint32, BatchDraws>> m_leafData;
    bool m_initialized;

    void addLeafData(uint32 materialFlags, uint32 meshId, DrawData draw);
    void removeLeafData(uint32 materialFlags);
    std::vector<Batch> getLeafData(uint32 materialFlags);

    void buildBranch(uint32 branchIndex, uint32 height);
    BatchNode* getBranch(uint32 branchIndex);
    void setBranch(uint32 branchIndex, BatchNode* branchData);

    uint32 subIndex(uint32 num, uint32 height);
    void setBit(uint32 bitIndex, uint32 bitValue);
    uint32 getBit(uint32 bitIndex);
};

}