#include "ResourceManager.h"

namespace spr{

ResourceManager::ResourceManager(){
    m_resourceCaches = std::vector<ResourceCache*>();
    m_resourceMap = rmap();
    m_resourceIndex = 0;
}

}