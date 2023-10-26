#pragma once

#include <vector>
#include "../Container.h"
#include "../../../ecs/Entity.h"

namespace spr {

class EntityNode {
public:
    EntityNode();
    EntityNode(uint32 height);
    ~EntityNode() {
        for (uint32 i = 0; i < m_nodeData.size(); i++){
            delete m_nodeData.at(i);
        }
    }

    //add
    void add(Entity& entity);
    //remove
    void remove(Entity& entity);
    //get
    Container<Entity> get(uint64 key);
    //get_accum
    void getAccum(uint64 key, std::vector<Entity>& out);

    uint32 getHeight(){
        return m_height;
    }

private:
    uint32 m_height;
    uint32 m_mask;
    std::vector<EntityNode*> m_nodeData;
    std::vector<Container<Entity>> m_leafData;
    bool m_initialized;

    void addLeafData(uint32 key, Entity& entity);
    void removeLeafData(uint32 key, Entity& entity);
    Container<Entity>& getLeafData(uint32 key);

    void buildBranch(uint32 branchIndex, uint32 height);
    EntityNode* getBranch(uint32 branchIndex);
    void setBranch(uint32 branchIndex, EntityNode* branchData);
    bool branchInitialized(uint32 branch);

    uint32 subIndex(uint64 num, uint32 height);
    void setBit(uint32 bitIndex, uint32 bitValue);
    uint32 getBit(uint32 bitIndex);
};

}