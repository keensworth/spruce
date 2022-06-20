#pragma once

namespace spr {
class Entity {
public:
    Entity();
    Entity(int id, long components);
    ~Entity() {}
    
    int id;
    long components;

    long addComponents(long components);
    long removeComponents(long components);
    bool contains(long components);
    bool containsAny(long components);

    bool operator==(const Entity &rhs) const {
        return (rhs.id == id)&(rhs.components == components);
    }

private:

};
}