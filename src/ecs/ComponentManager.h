#pragma once

#include <algorithm> 
#include <vector>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include "Component.h"


typedef std::unordered_map<std::type_index, int> tmap;

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
    int getComponentIndex(){
        int index = m_typeMap[typeid(T)];
        return index;
    }

    // get entity data for component T
    template <typename T>
    auto getEntityComponent(Entity entity){
        int index = m_typeMap[typeid(T)];
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        return comp->get(entity);
    }

    // set data for component T, associated with entity
    template <typename T>
    void setEntityComponent(Entity entity, auto data){
        int index = getComponentIndex<T>();
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->set(entity, data);
        
    }

    // add data for component T
    template <typename T>
    T* addComponentData(auto data){
        int index = getComponentIndex<T>();
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->add(data);
        return comp;
    }

    // add data for component T, associated with entity
    template <typename T>
    void addComponentEntityData(Entity entity, auto data){
        int index = getComponentIndex<T>();
        // update components
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->add(data);
        comp->addEntity(entity);
    }

    // remove component T associated with entity
    template <typename T>
    void removeComponentEntity(Entity entity){
        int index = getComponentIndex<T>();
        T* comp = ((T*) dynamic_cast<T*>(m_components.at(index)));
        comp->removeEntity(entity);
    }
    

    // get bit mask for arg components
    template <typename Arg, typename ...Args>
    long getMask(){
        long mask = 0L;
        int index = getComponentIndex<Arg>();
        
        mask |= 0b1 << index;
        
        if constexpr (sizeof...(Args) > 0){
            return (mask | getMask<Args...>());
        } else {
            return mask | getMask();
        }
    }

     // get bit mask for arg components
    long getMask(){
        return 0L;
    }

    // get bit mask for components vector
    long getMask(std::vector<Component*> components);

    // register entity with set of components
    template<typename T, typename... Args>
    void registerEntity(Entity entity, T* component, Args... args){
        registerEntityRecursive(entity, component, args...);
    }

    // register entity with set of components
    void unregisterEntity(Entity entity){
        long mask = entity.components;
        for (int i = 0; i < 64; i++){
            if ((mask>>i)&0b1 == 1){
                m_components.at(i)->removeEntity(entity);
            }
        }
    }


private:
    std::vector<Component*> m_components;
    tmap m_typeMap;
    tmap m_typeMapP;
    int m_componentIndex;

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