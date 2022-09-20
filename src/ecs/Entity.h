#pragma once

#include "../core/Core.h"

namespace spr {
class Entity {
public:
    Entity();
    Entity(uint32 id, uint64 components);
    ~Entity() {}
    
    uint32 id;
    uint64 components;

    uint64 addComponents(uint64 components);
    uint64 removeComponents(uint64 components);
    bool contains(uint64 components);
    bool containsAny(uint64 components);

    bool operator==(const Entity &rhs) const {
        return (rhs.id == id)&(rhs.components == components);
    }

private:

};
}