#include "ComponentManager.h"

namespace spr{

ComponentManager::ComponentManager(){
    m_components = std::vector<Component*>(64);
}

void ComponentManager::addComponent(Component& component){
    m_components.push_back(&component);
}

template <typename T>
bool typeMatch(auto data){
    return typeid(data)==typeid(T);
}

template <typename T>
auto ComponentManager::getEntityComponent(Entity entity){
    auto val = std::find_if(m_components.begin(), m_components.end(), typeMatch<T>());
    int index = -1;
    if (val != m_components.end()){
        index = std::distance(m_components.begin(), val);
        return m_components.at(index)->get(entity);
    } else {
        return nullptr;
    }
}

template <typename T>
void ComponentManager::setEntityComponent(Entity entity, auto data){
    auto val = std::find_if(m_components.begin(), m_components.end(), typeMatch<T>());
    int index = -1;
    if (val != m_components.end()){
        index = std::distance(m_components.begin(), val);
        m_components.at(index)->set(entity, data);
    }
}

template <typename T>
void ComponentManager::addComponentData(auto data){
    auto val = std::find_if(m_components.begin(), m_components.end(), typeMatch<T>());
    int index = -1;
    if (val != m_components.end()){
        index = std::distance(m_components.begin(), val);
        m_components.at(index)->add(data);
    }
}

template <typename T>
void ComponentManager::addComponentEntityData(Entity entity, auto data){
    auto val = std::find_if(m_components.begin(), m_components.end(), typeMatch<T>());
    int index = -1;
    if (val != m_components.end()){
        index = std::distance(m_components.begin(), val);
        m_components.at(index)->add(data);
        m_components.at(index)->addEntity(entity);
    }
}

template <typename T>
void ComponentManager::removeComponentEntity(Entity entity){
    auto val = std::find_if(m_components.begin(), m_components.end(), typeMatch<T>());
    int index = -1;
    if (val != m_components.end()){
        index = std::distance(m_components.begin(), val);
        m_components.at(index)->removeEntity(entity);
    }
}

template <typename Arg, typename ...Args>
long ComponentManager::getMask(){
    long mask = 0L;
    auto val = std::find_if(m_components.begin(), m_components.end(), typeMatch<T>());
    int index = -1;
    if (val != m_components.end()){
        index = std::distance(m_components.begin(), val);
        mask |= 0b1 << index;
        return (mask | getMask<Args>());
    } else {
        return getMask<Args>();
    }
}

long ComponentManager::getMask(std::vector<Component&> components){
    long mask = 0L;
    // for each component arg
    for (int i = 0; i < components.size(); i++){
        // for each registered component
        int j = 0;
        for (j = 0; j < m_components.size(); j++){
            // if components match, break
            if (typeid(components.at(i)) == typeid(m_components.at(j)))
                break;
            
        }
        // set mask bit relative to reg comp index
        if (j < m_components.size())
            mask |= (0b1 << j);
    }
    return mask;
}
}