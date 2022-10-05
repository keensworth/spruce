#include "ResourceManager.h"

namespace spr{

ResourceManager::ResourceManager(){
    m_resources = std::vector<ResourceCache*>();
    m_resourceMap = rmap();
    m_resourceIndex = 0;
}

}