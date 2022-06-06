#include "IndexNode.h"

namespace spr {

IndexNode::IndexNode(){
    m_height = 8;
    m_nodeData = std::vector<IndexNode*>(16);
    m_initialized = true;
}
IndexNode::IndexNode(int height){
    m_height = height;
    if (m_height > 0){
        m_nodeData = std::vector<IndexNode*>(16);
    } else {
        m_leafData = std::vector<int>(16);
    }
}


//add
void IndexNode::add(int key, int data){
    int currIndex;
    IndexNode* currNode = this;

    for (int height = m_height; height > 0; height--){
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
void IndexNode::remove(int key){
    int currIndex;
    IndexNode* currNode = this;

    for (int height = m_height; height > 0; height--){
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
int IndexNode::get(int key){
    int currIndex;
    IndexNode* currNode = this;

    for (int height = m_height; height > 0; height--){
        currIndex = subIndex(key, height);

        if (!currNode->getBranch(currIndex)->m_initialized){
            return NULL;
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(key, currNode->getHeight());
    return currNode->getLeafData(currIndex);
}

void IndexNode::addLeafData(int key, int data){
    m_leafData.at(key) = data;
}
void IndexNode::removeLeafData(int key){
    m_leafData.at(key) = -1;
}
int IndexNode::getLeafData(int key){
    return m_leafData.at(key);
}



void IndexNode::buildBranch(int branchIndex, int height){
    m_nodeData.at(branchIndex) = new IndexNode(height-1);
}

IndexNode* IndexNode::getBranch(int branchIndex){
    return m_nodeData.at(branchIndex);
}

void IndexNode::setBranch(int branchIndex, IndexNode* branchData){
    m_nodeData.at(branchIndex) = branchData;
}

int IndexNode::subIndex(int num, int height){
    return ((num>>(height*4))&0b1111);
}

void IndexNode::setBit(int bitIndex, int bitValue){
    int setterMask = ((0b00000001)<<bitIndex);
    if (bitValue==0){
        m_mask &= (setterMask^0xffffffff);
    } else {
        m_mask |= setterMask;
    }
}

int IndexNode::getBit(int bitIndex){
    return (m_mask>>bitIndex)&0b1;
}
}