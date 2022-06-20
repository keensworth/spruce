#include "ComponentManager.h"

namespace spr{

ComponentManager::ComponentManager(){
    m_components = std::vector<Component*>();
    m_typeMap = tmap();
    m_typeMapP = tmap();
    m_componentIndex = 0;
}

long ComponentManager::getMask(std::vector<Component*> components){
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