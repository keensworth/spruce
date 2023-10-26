#include "ComponentManager.h"

namespace spr{

ComponentManager::ComponentManager(){
    m_componentIndex = 0;
    m_trackingMask = 0;
}

uint64 ComponentManager::getMask(std::vector<Component*> components){
    uint64 mask = 0LL;
    // for each component arg
    for (int32 i = 0; i < components.size(); i++){
        // for each registered component
        int32 j = 0;
        for (j = 0; j < m_components.size(); j++){
            // if components match, break
            if (typeid(components.at(i)) == typeid(m_components.at(j)))
                break;
            
        }
        // set mask bit relative to reg comp index
        if (j < m_components.size())
            mask |= (1LL << j);
    }
    return mask;
}
}