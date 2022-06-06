#include "Component.h"

namespace spr {
template <typename T>
Component<T>::Component(){
    m_container = Container<T>(512);
    m_reg = Registry(512);
}

// get entity's component data
template <typename T>
T Component<T>::get(Entity entity){
    return m_container.at(m_reg.getIndex(entity));
}

// set entity's component data
template <typename T>
void Component<T>::set(Entity entity, T data){
    m_container.at(m_reg.getIndex(entity)) = data;
}

// add a new piece of data to container
template <typename T>
void Component<T>::add(T data){
    m_container.add(data);
}

// add entity to registry
template <typename T>
void Component<T>::addEntity(Entity entity){
    m_reg.addItem(entity.id, m_container.lastWriteIndex);
}

// remove entity from registry
template <typename T>
void Component<T>::removeEntity(Entity entity){
    // becasue the registry will not shrink when we remove
    // this does nothing for now
    //m_reg.(entity.id, m_container.lastWriteIndex);
}

}