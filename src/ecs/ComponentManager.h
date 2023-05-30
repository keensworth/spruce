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

    // add a component
    template <typename T>
    void addComponent(Component& component){
        m_components.push_back(&component);
        m_typeMap[typeid(T)] = m_componentIndex;
        m_typeMap[typeid(T*)] = m_componentIndex;
        m_componentIndex++;
    }

    template <typename T>
    uint32 getComponentIndex(){
        uint32 index = m_typeMap[typeid(T)];
        return index;
    }

    // get entity data for component T
    template <typename T>
    auto& getEntityComponent(Entity entity){
        uint32 index = m_typeMap[typeid(T)];
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        return comp->get(entity);
    }

    // set data for component T, associated with entity
    template <typename T>
    void setEntityComponent(Entity entity, auto data){
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
    void addComponentEntityData(Entity entity, auto data){
        uint32 index = getComponentIndex<T>();
        // update components
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->add(data);
        comp->addEntity(entity);
    }

    // remove component T associated with entity
    template <typename T>
    void removeComponentEntity(Entity entity){
        uint32 index = getComponentIndex<T>();
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->removeEntity(entity);
    }
    

    // get bit mask for arg components
    template <typename Arg, typename ...Args>
    uint64 getMask(){
        uint64 mask = 0L;
        uint32 index = getComponentIndex<Arg>();
        
        mask |= 0b1 << index;
        
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

    // register entity with set of components
    template<typename T, typename... Args>
    void registerEntity(Entity entity, T* component, Args... args){
        registerEntityRecursive(entity, component, args...);
    }

    // register entity with set of components
    void unregisterEntity(Entity entity){
        uint64 mask = entity.components;
        for (uint32 i = 0; i < 64; i++){
            if (((mask>>i)&0b1) == 1){
                m_components.at(i)->removeEntity(entity);
            }
        }
    }


private:
    std::vector<Component*> m_components;
    tmap m_typeMap;
    uint32 m_componentIndex;

    template<typename T, typename... Args>
    void registerEntityRecursive(Entity entity, T* component, Args... args){
        component->addEntity(entity);
        registerEntityRecursive(entity, args...);
    }

    void registerEntityRecursive(Entity entity){
        return;
    }
};
}