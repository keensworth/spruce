#pragma once

#include <vector>
#include "../core/util/node/EntityNode.h"

namespace spr {

class EntityManager {
public:
    EntityManager();
    ~EntityManager() {}

    void addEntity(Entity& entity);
    void removeEntity(Entity& entity);

    void update();

    void getEntities(uint64 components, std::vector<Entity>& out);
    
private:
    // entity storage
    EntityNode m_entities;

    // per-frame entity add/removal buffers
    std::vector<Entity> m_entitiesAdd;
    std::vector<Entity> m_entitiesRemove;

    void getEntities(){}
    void cleanUp();

    friend class SprECS;
};

}