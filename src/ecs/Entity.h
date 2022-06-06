#pragma once

namespace spr {
class Entity {
public:
    Entity(int id, long components);
    ~Entity() {}
    
    int id;
    long components;

    long addComponents(long components);
    long removeComponents(long components);
    bool contains(long components);
    bool containsAny(long components);

private:

};
}