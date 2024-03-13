#include "EntityManager.h"

namespace spr {

EntityManager::EntityManager() {}

void EntityManager::update(){
    if (m_entitiesAdd.size() > 0 || m_entitiesRemove.size() > 0){
        for (Entity& entity : m_entitiesAdd){
            m_entities.add(entity);
        }
        for (Entity& entity : m_entitiesRemove){
            m_entities.remove(entity);
        }
    }
}

void EntityManager::cleanUp(){
    if (m_entitiesAdd.size() > 0)
        m_entitiesAdd.clear();
    if (m_entitiesRemove.size() > 0)
        m_entitiesRemove.clear();
}

void EntityManager::addEntity(Entity& entity){
    m_entitiesAdd.push_back(entity);
}

void EntityManager::removeEntity(Entity& entity){
    m_entitiesRemove.push_back(entity);
}

void EntityManager::getEntities(uint64 components, std::vector<Entity>& out){
    m_entities.getAccum(components, out);
}

}