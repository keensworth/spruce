#include "Entity.h"

namespace spr {
Entity::Entity(int id, long components){
    this->id = id;
    this->components = components;
}

long Entity::addComponents(long componentMask){
    components |= componentMask;
    return components;
}

long Entity::removeComponents(long componentMask){
    components &= (~componentMask);
    return components;
}   

bool Entity::contains(long componentMask){
    return ((components & componentMask) == componentMask);
}

bool Entity::containsAny(long componentMask){
    return ((components & componentMask) != 0);
}
}