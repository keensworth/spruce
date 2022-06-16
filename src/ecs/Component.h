#pragma once

#include "Registry.h"
#include "../core/util/Container.h"

namespace spr {

class Component {
public:
    // get entity's component data
    auto get(Entity entity);

    // set entity's component data
    void set(Entity entity, auto data);

    // add a new piece of data to container
    void add(auto data);

    // add entity to registry
    void addEntity(Entity entity);

    // remove entity from registry
    void removeEntity(Entity entity);
};

template <typename T>
class TypedComponent : public Component{
public:
    TypedComponent();
    ~TypedComponent() {}

    // get entity's component data
    T get(Entity entity);

    // set entity's component data
    void set(Entity entity, T data);

    // add a new piece of data to container
    void add(T data);

    // add entity to registry
    void addEntity(Entity entity);

    // remove entity from registry
    void removeEntity(Entity entity);

private:
    // Per entity data
    Container<T> m_container;
    // Entity -> index map
    Registry m_reg;
};

}