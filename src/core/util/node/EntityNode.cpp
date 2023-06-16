#include "EntityNode.h"

namespace spr {

EntityNode::EntityNode(){
    m_height = 16;
    m_nodeData = std::vector<EntityNode*>(16);
    m_initialized = true;
    m_mask = 0;
}
EntityNode::EntityNode(uint32 height){
    m_height = height;
    if (m_height > 0){
        m_nodeData = std::vector<EntityNode*>(16);
    } else {
        m_leafData = std::vector<Container<Entity>>(16);
    }
    m_initialized = true;
    m_mask = 0;
}


//add
void EntityNode::add(Entity entity){
    uint64 currIndex;
    EntityNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(entity.components, height);
        
        if (!currNode->branchInitialized(currIndex)){
            currNode->buildBranch(currIndex, height);
            currNode->setBit(currIndex,1);
        }
        
        currNode = currNode->getBranch(currIndex);
        currIndex = 0;
    }
    currIndex = subIndex(entity.components, currNode->getHeight());
    currNode->addLeafData(currIndex, entity);
}

//remove
void EntityNode::remove(Entity entity){
    uint32 currIndex;
    EntityNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(entity.components, height);

        if (currNode->branchInitialized(currIndex)){
            currNode = currNode->getBranch(currIndex);
        }
    }

    currIndex = subIndex(entity.components, currNode->getHeight());
    currNode->removeLeafData(currIndex, entity);
}

//get
Container<Entity> EntityNode::get(uint64 key){
    uint32 currIndex;
    EntityNode* currNode = this;

    for (uint32 height = m_height; height > 0; height--){
        currIndex = subIndex(key, height);

        if (!currNode->branchInitialized(currIndex)){
            return Container<Entity>(0);
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(key, currNode->getHeight());
    return currNode->getLeafData(currIndex);
}

void EntityNode::getAccum(uint64 key, Container<Entity>& out){
    uint32 currIndex = subIndex(key, m_height);
    for (uint32 i = 0; i < 16; i++){
        if (m_height > 0){
            if (!((currIndex & i) == currIndex && branchInitialized(i))){
                continue;
            }
            EntityNode* node = getBranch(i);
            node->getAccum(key, out);
        } else {
            if (!((currIndex & i) == currIndex && m_initialized)){
                continue;
            }
            out.append(getLeafData(i));
        }
    }
}

void EntityNode::addLeafData(uint32 key, Entity entity){
    m_leafData.at(key).add(entity);
}
void EntityNode::removeLeafData(uint32 key, Entity entity){
    m_leafData.at(key).eraseData(entity);
}
Container<Entity>& EntityNode::getLeafData(uint32 key){
    return m_leafData.at(key);
}


void EntityNode::buildBranch(uint32 branchIndex, uint32 height){
    m_nodeData.at(branchIndex) = new EntityNode(height-1);
}

EntityNode* EntityNode::getBranch(uint32 branchIndex){
    return m_nodeData.at(branchIndex);
}

bool EntityNode::branchInitialized(uint32 branch){
    return (m_mask >> branch)&0b1;
}

void EntityNode::setBranch(uint32 branchIndex, EntityNode* branchData){
    m_nodeData.at(branchIndex) = branchData;
}

uint32 EntityNode::subIndex(uint64 num, uint32 height){
    return ((num>>(height*4))&0b1111);
}

void EntityNode::setBit(uint32 bitIndex, uint32 bitValue){
    uint32 setterMask = ((0b00000001)<<bitIndex);
    if (bitValue==0){
        m_mask &= (setterMask^0xffffffff);
    } else {
        m_mask |= setterMask;
    }
}

uint32 EntityNode::getBit(uint32 bitIndex){
    return (m_mask>>bitIndex)&0b1;
}
}