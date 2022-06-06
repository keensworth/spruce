#pragma once

#include <Container.h>
#include <Registry.h>

namespace spr {
template <typename T>
class Component {
public:
    Component();
    ~Component() {}

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