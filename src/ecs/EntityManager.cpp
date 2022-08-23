#include "EntityManager.h"

namespace spr {

EntityManager::EntityManager(){
    m_entities = EntityNode();
    m_entitiesAdd = std::vector<Entity>();
    m_entitiesRemove = std::vector<Entity>();
}

void EntityManager::update(){
    if (m_entitiesAdd.size() > 0 || m_entitiesRemove.size() > 0){
        for (Entity entity : m_entitiesAdd){
            m_entities.add(entity);
        }
        for (Entity entity : m_entitiesRemove){
            m_entities.remove(entity);
        }
        if (m_entitiesAdd.size() > 0)
            m_entitiesAdd.clear();
        if (m_entitiesRemove.size() > 0)
            m_entitiesRemove.clear();
    }
}

void EntityManager::addEntity(Entity entity){
    m_entitiesAdd.push_back(entity);
}

void EntityManager::removeEntity(Entity entity){
    m_entitiesRemove.push_back(entity);
}

Container<Entity> EntityManager::getEntities(uint64 components){
    Container<Entity> cont = m_entities.getAccum(components);
    return cont;
}

}