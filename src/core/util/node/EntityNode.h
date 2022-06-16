#pragma once

#include <vector>
#include "../../../ecs/Entity.h"
#include "../Container.h"

namespace spr {

class EntityNode {
public:
    EntityNode();
    EntityNode(int height);
    ~EntityNode() {
        for (int i = 0; i < m_nodeData.size(); i++){
            delete m_nodeData.at(i);
        }
    }

    //add
    void add(Entity entity);
    //remove
    void remove(Entity entity);
    //get
    Container<Entity> get(long key);
    //get_accum
    Container<Entity> getAccum(long key);

    int getHeight(){
        return m_height;
    }

private:
    int m_height;
    int m_mask;
    std::vector<EntityNode*> m_nodeData;
    std::vector<Container<Entity>> m_leafData;
    bool m_initialized;

    void addLeafData(int key, Entity entity);
    void removeLeafData(int key, Entity entity);
    Container<Entity> getLeafData(int key);
    Container<Entity> getAccumLeafData(int key);

    void buildBranch(int branchIndex, int height);
    EntityNode* getBranch(int branchIndex);
    void setBranch(int branchIndex, EntityNode* branchData);

    int subIndex(long num, int height);
    void setBit(int bitIndex, int bitValue);
    int getBit(int bitIndex);
};

}