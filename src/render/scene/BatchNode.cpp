#include <cmath>
#include "BatchNode.h"


namespace spr::gfx {

BatchNode::BatchNode(){
    m_height = 8;
    m_nodeData = std::vector<BatchNode*>(16);
    m_initialized = true;
}
BatchNode::BatchNode(uint32 height){
    m_height = height;
    if (m_height > 0){
        m_nodeData = std::vector<BatchNode*>(16);
    } else {
        m_leafData = std::vector<ska::flat_hash_map<uint32, BatchDraws>>(16);
    }
}


//add
void BatchNode::add(uint32 materialFlags, uint32 meshId, DrawData draw){
    uint32 currIndex;
    BatchNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(materialFlags, height);

        if (!currNode->getBranch(currIndex)->m_initialized){
            currNode->buildBranch(currIndex, height);
            currNode->setBit(currIndex,1);
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(materialFlags, currNode->getHeight());
    currNode->addLeafData(currIndex, meshId, draw);
}

//remove
void BatchNode::remove(uint32 materialFlags){
    uint32 currIndex;
    BatchNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(materialFlags, height);

        if (!currNode->getBranch(currIndex)->m_initialized){
            return;
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(materialFlags, currNode->getHeight());
    currNode->removeLeafData(currIndex);
}

//get
std::vector<Batch> BatchNode::get(uint32 materialFlags){
    uint32 currIndex;
    BatchNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(materialFlags, height);

        if (!currNode->getBranch(currIndex)->m_initialized){
            return std::vector<Batch>();
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(materialFlags, currNode->getHeight());
    return currNode->getLeafData(currIndex);
}

std::vector<Batch> BatchNode::getAny(uint32 materialFlags){
    uint32 currIndex = subIndex(materialFlags, m_height);
    std::vector<Batch> accum;
    accum.reserve((m_height+1)*256);
    for (uint32 i = 0; i < 16; i++){
        if (m_height > 0){
            if (!((((currIndex & i) != 0) || ((currIndex & i) == currIndex)) && getBranch(i)->m_initialized)){
                continue;
            }
            BatchNode* node = getBranch(i);
            std::vector<Batch> batches = node->getAny(materialFlags);
            accum.insert(accum.end(), batches.begin(), batches.end());
        } else {
            if (!((((currIndex & i) != 0) || ((currIndex & i) == currIndex)) && m_initialized)){
                continue;
            }
            std::vector<Batch> batches = getLeafData(i);
            accum.insert(accum.end(), batches.begin(), batches.end());
        }
    }

    return accum;
}

// void BatchNode::uploadDrawData(){
//     if (!getBranch(i)->m_initialized)
//                 continue;
            
//             uploadDrawDataRec(rm, getBranch(i)); // go down correct branch
// }

// void BatchNode::uploadDrawDataRec(BatchNode* batchNode){
//     for (uint32 i = 0; i < 16; i++){
//         if (m_height > 0){
//             if (!getBranch(i)->m_initialized)
//                 continue;
            
//             uploadDrawDataRec(getBranch(i), rm); // go down correct branch
            
//         } else {
//             if (!m_initialized)
//                 continue;

//             for(auto item : m_leafData[i]){
//                 BatchDraws& batchDraws = item.second;
//                 uint32 drawDataOffset = rm->addDrawData(batchDraws.draws); // TODO: bump allocator
//                 batchDraws.batch.drawDataOffset = drawDataOffset;
//                 batchDraws.batch.drawCount = batchDraws.draws.size();
//             }
//         }
//     }
// }



void BatchNode::addLeafData(uint32 materialFlags, uint32 meshId, DrawData draw){
    if (!m_leafData.at(materialFlags).count(meshId)) {
        m_leafData.at(materialFlags)[meshId] = BatchDraws();
        BatchDraws& batchDraws = m_leafData.at(materialFlags)[meshId];
        batchDraws.batch = Batch();
        batchDraws.batch.meshId = meshId;
        batchDraws.batch.materialFlags = materialFlags;
        return;
    }

    m_leafData.at(materialFlags)[meshId].draws.push_back(draw);
}

void BatchNode::removeLeafData(uint32 key){
    m_leafData.at(key) = ska::flat_hash_map<uint32, BatchDraws>();
}

std::vector<Batch> BatchNode::getLeafData(uint32 key){
    std::vector<Batch> batches;
    for(auto item : m_leafData.at(key))
        batches.push_back(item.second.batch);
    return batches;
}


void BatchNode::buildBranch(uint32 branchIndex, uint32 height){
    m_nodeData.at(branchIndex) = new BatchNode(height-1);
}

BatchNode* BatchNode::getBranch(uint32 branchIndex){
    return m_nodeData.at(branchIndex);
}

void BatchNode::setBranch(uint32 branchIndex, BatchNode* branchData){
    m_nodeData.at(branchIndex) = branchData;
}

uint32 BatchNode::subIndex(uint32 num, uint32 height){
    return ((num>>(height*4))&0b1111);
}

void BatchNode::setBit(uint32 bitIndex, uint32 bitValue){
    uint32 setterMask = ((0b00000001)<<bitIndex);
    if (bitValue==0){
        m_mask &= (setterMask^0xffffffff);
    } else {
        m_mask |= setterMask;
    }
}

uint32 BatchNode::getBit(uint32 bitIndex){
    return (m_mask>>bitIndex)&0b1;
}
}