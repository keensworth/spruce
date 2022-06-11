#include "EntityNode.h"

namespace spr {

EntityNode::EntityNode(){
    m_height = 16;
    m_nodeData = std::vector<EntityNode*>(16);
    m_initialized = true;
}
EntityNode::EntityNode(int height){
    m_height = height;
    if (m_height > 0){
        m_nodeData = std::vector<EntityNode*>(16);
    } else {
        m_leafData = std::vector<Container<Entity>>(16);
    }
}


//add
void EntityNode::add(Entity entity){
    long currIndex;
    EntityNode* currNode = this;

    for (int height = m_height; height > 0; height--){
        currIndex = subIndex(entity.components, height);

        if (!currNode->getBranch(currIndex)->m_initialized){
            currNode->buildBranch(currIndex, height);
            currNode->setBit(currIndex,1);
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(entity.components, currNode->getHeight());
    currNode->addLeafData(currIndex, entity);
}

//remove
void EntityNode::remove(Entity entity){
    int currIndex;
    EntityNode* currNode = this;

    for (int height = m_height; height > 0; height--){
        currIndex = subIndex(entity.components, height);

        if (!currNode->getBranch(currIndex)->m_initialized){
            return;
        }

        currNode = currNode->getBranch(currIndex);
    }

    currIndex = subIndex(entity.components, currNode->getHeight());
    currNode->removeLeafData(currIndex, entity);
}

//get
Container<Entity> EntityNode::get(long key){
    int currIndex;
    EntityNode* currNode = this;

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

Container<Entity> EntityNode::getAccum(long key){
    int currIndex = subIndex(key, m_height);
    Container<Entity> accum;

    for (int i = 0; i < 16; i++){
        if (m_height > 0){
            if (!((currIndex & i) == currIndex && m_nodeData.at(i)->m_initialized)){
                continue;
            }
            EntityNode* node = getBranch(i);
            Container<Entity> entities = node->getAccum(key);
            accum.append(entities);
        } else {
            if (!((currIndex & i) == currIndex && m_initialized)){
                continue;
            }
            Container<Entity> entities = getLeafData(i);
            accum.append(entities);
        }
    }

    return accum;
}

void EntityNode::addLeafData(int key, Entity entity){
    m_leafData.at(key).add(entity);
}
void EntityNode::removeLeafData(int key, Entity entity){
    m_leafData.at(key).remove(entity);
}
Container<Entity> EntityNode::getLeafData(int key){
    return m_leafData.at(key);
}
Container<Entity> EntityNode::getAccumLeafData(int key){
    return m_leafData.at(key);
}


void EntityNode::buildBranch(int branchIndex, int height){
    m_nodeData.at(branchIndex) = new EntityNode(height-1);
}

EntityNode* EntityNode::getBranch(int branchIndex){
    return m_nodeData.at(branchIndex);
}

void EntityNode::setBranch(int branchIndex, EntityNode* branchData){
    m_nodeData.at(branchIndex) = branchData;
}

int EntityNode::subIndex(long num, int height){
    return ((num>>(height*4))&0b1111);
}

void EntityNode::setBit(int bitIndex, int bitValue){
    int setterMask = ((0b00000001)<<bitIndex);
    if (bitValue==0){
        m_mask &= (setterMask^0xffffffff);
    } else {
        m_mask |= setterMask;
    }
}

int EntityNode::getBit(int bitIndex){
    return (m_mask>>bitIndex)&0b1;
}
}