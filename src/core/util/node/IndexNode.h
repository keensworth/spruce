#pragma once

#include <vector>
#include "../../Core.h"

namespace spr {

class IndexNode {
public:
    IndexNode();
    IndexNode(uint32 height);
    ~IndexNode() {
        for (uint32 i = 0; i < m_nodeData.size(); i++){
            delete m_nodeData.at(i);
        }
    }

    //add
    void add(uint32 key, int32 data);
    //remove
    void remove(uint32 key);
    //get
    int32 get(uint32 key);
    //get_accum

    uint32 getHeight(){
        return m_height;
    }

private:
    uint32 m_height;
    uint32 m_mask;
    std::vector<IndexNode*> m_nodeData;
    std::vector<int32> m_leafData;
    bool m_initialized;

    void addLeafData(uint32 key, int32 data);
    void removeLeafData(uint32 key);
    int32 getLeafData(uint32 key);

    void buildBranch(uint32 branchIndex, uint32 height);
    IndexNode* getBranch(uint32 branchIndex);
    void setBranch(uint32 branchIndex, IndexNode* branchData);

    uint32 subIndex(uint32 num, uint32 height);
    void setBit(uint32 bitIndex, uint32 bitValue);
    uint32 getBit(uint32 bitIndex);
};

}