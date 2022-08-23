#include "Entity.h"

namespace spr {
Entity::Entity(){
    id = 0;
    components = 0;
}

Entity::Entity(uint32 id, uint64 components){
    this->id = id;
    this->components = components;
}

uint64 Entity::addComponents(uint64 componentMask){
    components |= componentMask;
    return components;
}

uint64 Entity::removeComponents(uint64 componentMask){
    components &= (~componentMask);
    return components;
}   

bool Entity::contains(uint64 componentMask){
    return ((components & componentMask) == componentMask);
}

bool Entity::containsAny(uint64 componentMask){
    return ((components & componentMask) != 0);
}
}