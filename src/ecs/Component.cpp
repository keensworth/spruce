#include "Component.h"

namespace spr {
template <typename T>
TypedComponent<T>::TypedComponent(){
    m_container = Container<T>(512);
    m_reg = Registry(512);
}

// get entity's component data
template <typename T>
T TypedComponent<T>::get(Entity entity){
    return m_container.at(m_reg.getIndex(entity));
}

// set entity's component data
template <typename T>
void TypedComponent<T>::set(Entity entity, T data){
    m_container.at(m_reg.getIndex(entity)) = data;
}

// add a new piece of data to container
template <typename T>
void TypedComponent<T>::add(T data){
    m_container.add(data);
}

// add entity to registry
template <typename T>
void TypedComponent<T>::addEntity(Entity entity){
    m_reg.addItem(entity.id, m_container.lastWriteIndex);
}

// remove entity from registry
template <typename T>
void TypedComponent<T>::removeEntity(Entity entity){
    // free container slot
    m_container.remove(m_reg.getIndex(entity.id));
}

}