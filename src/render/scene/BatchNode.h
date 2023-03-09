#pragma once

#include <vector>
#include "spruce_core.h"
#include "../../../external/flat_hash_map/flat_hash_map.hpp"
#include "../vulkan/resource/VulkanResourceManager.h"
#include "Draw.h"

namespace spr::gfx {

class BatchNode {
public:
    BatchNode();
    BatchNode(uint32 height);
    ~BatchNode() {
        for (uint32 i = 0; i < m_nodeData.size(); i++){
            delete m_nodeData.at(i);
        }
    }

    void add(DrawData draw, Batch batchInfo);
    void remove(uint32 materialFlags);
    void get(BatchMaterialQuery query, std::vector<DrawBatch>& result);

    uint32 getHeight(){
        return m_height;
    }

private:
    uint32 m_height;
    uint32 m_mask;
    std::vector<BatchNode*> m_nodeData;
    std::vector<ska::flat_hash_map<uint32, DrawBatch>> m_leafData;
    bool m_initialized;

    typedef struct QueryType {
        bool usesHasAll;
        bool usesHasAny;
        bool usesHasExactly;
        bool usesExcludes;
    } QueryType;

    void getRec(BatchMaterialQuery query, std::vector<DrawBatch>& result, QueryType queryType);

    void addLeafData(uint32 branchIndex, DrawData draw, Batch batchInfo);
    void removeLeafData(uint32 branchIndex);
    void getLeafData(uint32 branchIndex, std::vector<DrawBatch>& dst);

    void buildBranch(uint32 branchIndex, uint32 height);
    BatchNode* getBranch(uint32 branchIndex);
    void setBranch(uint32 branchIndex, BatchNode* branchData);

    uint32 subIndex(uint32 num, uint32 height);
    void setBit(uint32 bitIndex, uint32 bitValue);
    uint32 getBit(uint32 bitIndex);
};

}