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
    virtual void addEntity(Entity entity) = 0;

    // remove entity from registry
    virtual void removeEntity(Entity entity) = 0;
};

template <typename T>
class TypedComponent : public Component{
public:
    TypedComponent(){
        m_container = Container<T>();
        m_reg = Registry(512);
        explicitInit = true;
    }

    ~TypedComponent(){}

    // get entity's component data
    T get(Entity entity) {
        return m_container.get(m_reg.getIndex(entity));
    }

    // set entity's component data
    void set(Entity entity, T data){
        m_container.set(m_reg.getIndex(entity), data);
    }

    // add a new piece of data to container
    void add(T data){
        m_container.add(data);
    }

    // add entity to registry
    void addEntity(Entity entity){
        m_reg.addItem(entity.id, m_container.lastWriteIndex);
    }

    // remove entity from registry
    void removeEntity(Entity entity){
        // free container slot
        m_container.erase(m_reg.getIndex(entity.id));
    }

    int32 getSize(){
        return m_reg.getSize();
    }

private:
    // Per entity data
    Container<T> m_container;
    // Entity -> index map
    Registry m_reg;
    bool explicitInit = false;
};

}