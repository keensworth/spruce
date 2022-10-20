// Handle <---> Data (*)

// Independantly load buffers/textures

// data not there get shit on LOL (checks cache, ensures loaded)

#pragma once

#include "ResourceCache.h"


typedef std::unordered_map<std::type_index, uint32> rmap;

namespace spr {
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager(){}

    template <typename T>
    void addResourceType(ResourceCache& resourceCache){
        m_resourceCaches.push_back(&resourceCache);
        m_resourceMap[typeid(T)] = m_resourceIndex;
        m_resourceMap[typeid(T*)] = m_resourceIndex;
        m_resourceIndex++;
    }

    // T := ResourceCache
    template <typename T>
    uint32 getResourceCacheIndex(){
        uint32 index = m_resourceMap[typeid(T)];
        return index;
    }

    // T := ResourceCache
    template <typename T>
    void registerResource(ResourceMetadata metadata){
        uint32 index = getResourceCacheIndex<T>();
        T* resourceCache = ((T*) dynamic_cast<T*>(m_resourceCaches.at(index)));
        resourceCache->registerResource(metadata);
    }

    // T := ResourceCache
    template <typename T>
    auto getHandle(uint32 id){
        uint32 index = getResourceCacheIndex<T>();
        T* resourceCache = ((T*) dynamic_cast<T*>(m_resourceCaches.at(index)));
        return resourceCache->getHandle(id);
    }

    // T := ResourceCache
    // U := ResourceInstance (not explicit, return only)
    template <typename T, typename U>
    U* getData(Handle<U> handle){
        uint32 index = getResourceCacheIndex<T>();
        T* resourceCache = ((T*) dynamic_cast<T*>(m_resourceCaches.at(index)));
        return resourceCache->getData(handle);
    }

private:
    std::vector<ResourceCache*> m_resourceCaches;
    rmap m_resourceMap;
    uint32 m_resourceIndex;
};
}