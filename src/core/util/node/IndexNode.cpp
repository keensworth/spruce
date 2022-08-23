#include "IndexNode.h"

namespace spr {

IndexNode::IndexNode(){
    m_height = 8;
    m_nodeData = std::vector<IndexNode*>(16);
    m_initialized = true;
}
IndexNode::IndexNode(uint32 height){
    m_height = height;
    if (m_height > 0){
        m_nodeData = std::vector<IndexNode*>(16);
    } else {
        m_leafData = std::vector<int32>(16);
    }
}


//add
void IndexNode::add(uint32 key, int32 data){
    uint32 currIndex;
    IndexNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(key, height);

        if (!currNode->getBranch(currIndex)->m_initialized){
            currNode->buildBranch(currIndex, height);
            currNode->setBit(currIndex,1);
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(key, currNode->getHeight());
    currNode->addLeafData(currIndex, data);
}

//remove
void IndexNode::remove(uint32 key){
    uint32 currIndex;
    IndexNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(key, height);

        if (!currNode->getBranch(currIndex)->m_initialized){
            return;
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(key, currNode->getHeight());
    currNode->removeLeafData(currIndex);
}

//get
int32 IndexNode::get(uint32 key){
    uint32 currIndex;
    IndexNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(key, height);

        if (!currNode->getBranch(currIndex)->m_initialized){
            return -1;
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(key, currNode->getHeight());
    return currNode->getLeafData(currIndex);
}

void IndexNode::addLeafData(uint32 key, int32 data){
    m_leafData.at(key) = data;
}
void IndexNode::removeLeafData(uint32 key){
    m_leafData.at(key) = -1;
}
int32 IndexNode::getLeafData(uint32 key){
    return m_leafData.at(key);
}



void IndexNode::buildBranch(uint32 branchIndex, uint32 height){
    m_nodeData.at(branchIndex) = new IndexNode(height-1);
}

IndexNode* IndexNode::getBranch(uint32 branchIndex){
    return m_nodeData.at(branchIndex);
}

void IndexNode::setBranch(uint32 branchIndex, IndexNode* branchData){
    m_nodeData.at(branchIndex) = branchData;
}

uint32 IndexNode::subIndex(uint32 num, uint32 height){
    return ((num>>(height*4))&0b1111);
}

void IndexNode::setBit(uint32 bitIndex, uint32 bitValue){
    uint32 setterMask = ((0b00000001)<<bitIndex);
    if (bitValue==0){
        m_mask &= (setterMask^0xffffffff);
    } else {
        m_mask |= setterMask;
    }
}

uint32 IndexNode::getBit(uint32 bitIndex){
    return (m_mask>>bitIndex)&0b1;
}
}