#pragma once

#include <algorithm> 
#include <vector>
#include "Component.h"
#include "Entity.h"


namespace spr {
class ComponentManager{
public:
    ComponentManager();
    ~ComponentManager() {}

    // add a component
    void addComponent(Component& component);

    // get entity data for component T
    template <typename T>
    auto getEntityComponent(Entity entity);

    // set data for component T, associated with entity
    template <typename T>
    void setEntityComponent(Entity entity, auto data);

    // add data for component T
    template <typename T>
    void addComponentData(auto data);

    // add data for component T, associated with entity
    template <typename T>
    void addComponentEntityData(Entity entity, auto data);

    // remove component T associated with entity
    template <typename T>
    void removeComponentEntity(Entity entity);

    // get bit mask for arg components
    template <typename Arg, typename ...Args>
    long getMask();

    // get bit mask for components vector
    long getMask(std::vector<Component&> components);

    // register entity with set of components
    void registerEntity(Entity entity, std::vector<Component&> components);

    // register entity with set of components
    void unregisterEntity(Entity entity);


private:
    std::vector<Component*> m_components;

    // getMask variadic base case
    long getMask(){
        return 0L;
    }
};
}