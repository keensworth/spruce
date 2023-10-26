#pragma once

#include <algorithm> 
#include <vector>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include "Component.h"


typedef std::unordered_map<std::type_index, uint32> tmap;

namespace spr {
class ComponentManager{
public:
    ComponentManager();
    ~ComponentManager() {}

    void update(){
        for (uint32 i = 0; i < m_componentIndex; i++)
            (m_components.at(i))->update();
        
        for (Entity& entity : m_entitiesUnregister)
            unregisterEntityImmediate(entity);
        
        if (m_entitiesUnregister.size() > 0)
            m_entitiesUnregister.clear();
    }

    // add a component
    template <typename T>
    void addComponent(Component& component, bool enableDirtyFlagTracking = false){
        m_components.push_back(&component);
        m_typeMap[typeid(T)] = m_componentIndex;
        m_typeMap[typeid(T*)] = m_componentIndex;

        if (enableDirtyFlagTracking){
            T* comp = ((T*) dynamic_cast<T*>(m_components.at(m_componentIndex)));
            comp->trackDirty();
            m_trackingMask |= 1LL << m_componentIndex;
        }

        m_componentIndex++;
    }

    template <typename T>
    uint32 getComponentIndex(){
        uint32 index = m_typeMap[typeid(T)];
        return index;
    }

    // get component object
    template <typename T>
    T* getComponent(){
        uint32 index = m_typeMap[typeid(T)];
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        return comp;
    }

    // get entity data for component T
    template <typename T>
    auto& getEntityComponent(Entity& entity){
        uint32 index = m_typeMap[typeid(T)];
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        return comp->get(entity);
    }

    // set data for component T, associated with entity
    template <typename T>
    void setEntityComponent(Entity& entity, auto data){
        uint32 index = getComponentIndex<T>();
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->set(entity, data);
    }

    // add data for component T
    template <typename T>
    T* addComponentData(auto data){
        uint32 index = getComponentIndex<T>();
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->add(data);
        return comp;
    }

    // add data for component T, associated with entity
    template <typename T>
    void addComponentEntityData(Entity& entity, auto data){
        uint32 index = getComponentIndex<T>();
        // update components
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->add(data);
        comp->addEntity(entity);
    }

    // remove component T associated with entity
    template <typename T>
    void removeComponentEntity(Entity& entity){
        uint32 index = getComponentIndex<T>();
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->removeEntity(entity);
    }
    

    // get bit mask for arg components
    template <typename Arg, typename ...Args>
    uint64 getMask(){
        uint64 mask = 0L;
        uint32 index = getComponentIndex<Arg>();
        
        mask |= 1LL << index;
        
        if constexpr (sizeof...(Args) > 0){
            return (mask | getMask<Args...>());
        } else {
            return mask | getMask();
        }
    }

     // get bit mask for arg components
    uint64 getMask(){
        return 0L;
    }

    // get bit mask for components vector
    uint64 getMask(std::vector<Component*> components);

    uint64 getTrackingMask(){
        return m_trackingMask;
    }

    // filter entities for those with a set dirty flag on given component
    template <typename T>
    void getDirtyEntities(std::vector<Entity>& in, std::vector<Entity>& out){
        uint32 index = m_typeMap[typeid(T)];
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));

        for (Entity& entity : in){
            if (!comp->isDirty(entity))
                out.push_back(entity);
        }
    }

    // check if an entity's component is dirty
    template <typename T>
    bool isDirty(Entity& entity){
        uint32 index = m_typeMap[typeid(T)];
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));

        if (comp->isDirty(entity.id))
            return true;
        
        return false;
    }

    // register entity with set of components
    template<typename T, typename... Args>
    void registerEntity(Entity& entity, T* component, Args... args){
        registerEntityRecursive(entity, component, args...);
    }

    void unregisterEntity(Entity& entity){
        m_entitiesUnregister.push_back(entity);
    }


private:
    std::vector<Component*> m_components;
    tmap m_typeMap;
    uint32 m_componentIndex;
    uint64 m_trackingMask;

    std::vector<Entity> m_entitiesUnregister;

    template<typename T, typename... Args>
    void registerEntityRecursive(Entity& entity, T* component, Args... args){
        component->addEntity(entity);
        registerEntityRecursive(entity, args...);
    }

    void registerEntityRecursive(Entity& entity){
        return;
    }

    void unregisterEntityImmediate(Entity& entity){
        uint64 mask = entity.components;
        for (uint32 i = 0; i < 64; i++){
            if (((mask>>i)&0b1) == 1){
                m_components.at(i)->removeEntity(entity);
            }
        }
    }
};
}