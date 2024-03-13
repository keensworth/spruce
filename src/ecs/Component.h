#pragma once

#include "Registry.h"
#include "../core/util/Container.h"
#include <vector>

namespace spr {

class Component {
public:
    // get entity's component data
    auto& get(Entity& entity);

    // set entity's component data
    void set(Entity& entity, auto data);

    // add a new piece of data to container
    void add(auto data);

    // add entity to registry
    virtual void addEntity(Entity& entity) = 0;

    // remove entity from registry
    virtual void removeEntity(Entity& entity) = 0;

    virtual void update() = 0;
};


template <typename T>
class TypedComponent : public Component{
public:
    TypedComponent() : m_container(512), m_reg(512){}

    ~TypedComponent(){}

    // get entity's component data
    T& get(Entity& entity) {
        return m_container[m_reg.getIndex(entity)];
    }

    T& get(uint32 id) {
        return m_container[m_reg.getIndex(id)];
    }

    // set entity's component data
    void set(Entity& entity, T data){
        m_container.set(m_reg.getIndex(entity), data);

        if (m_trackDirty)
            dirty(entity.id);
    }

    // add a new piece of data to container
    void add(T data){
        m_container.add(data);
    }

    // add entity to registry
    void addEntity(Entity& entity){
        m_reg.addItem(entity.id, m_container.lastWriteIndex);

        // if (m_trackDirty)
        //     dirty(entity.id);
    }

    // remove entity from registry
    void removeEntity(Entity& entity){
        // free container slot
        m_container.erase(m_reg.getIndex(entity.id));
    }

    uint32 size(){
        return m_reg.size();
    }

    bool isDirty(uint32 id){
        if (!m_trackDirty)
            return false;

        while (id / 64 >= m_dirty.size()){
            m_dirty.push_back(0);
        }

        uint64 dirtyMask = m_dirty[id / 64];
        if (!dirtyMask)
            return false;
        return dirtyMask & (1LL << (id % 64));
    }

    std::vector<uint32>& getDirtyIds(){
        return m_dirtyIds;
    }

    void update(){
        if (!m_trackDirty)
            return;

        std::fill(m_dirty.begin(), m_dirty.end(), 0);
        m_dirtyIds.clear();
    }

    void clean(uint32 id){
        if (!m_trackDirty)
            return;

        while (id / 64 >= m_dirty.size()){
            m_dirty.push_back(0);
        }

        m_dirty[id / 64] &= ~(1LL << (id % 64));
    }

    void dirty(uint32 id){
        if (!m_trackDirty)
            return;

        while (id / 64 >= m_dirty.size()){
            m_dirty.push_back(0);
        }

        m_dirty[id / 64] |= (1LL << (id % 64));

        m_dirtyIds.push_back(id);
    }
    
private:
    void trackDirty(){
        m_trackDirty = true;
        m_dirty = std::vector<uint64>(16);
        m_dirtyIds.reserve(64);
    }
    friend class ComponentManager;

private:
    // per entity data
    Container<T> m_container;

    // entity -> index map
    Registry m_reg;

    // dirty flags for new/edited components
    bool m_trackDirty = false;
    std::vector<uint64> m_dirty;
    std::vector<uint32> m_dirtyIds;
};

}