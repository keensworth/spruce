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
    void addResourceType(ResourceCache& resource){
        m_resources.push_back(&resource);
        m_resourceMap[typeid(T)] = m_resourceIndex;
        m_resourceMap[typeid(T*)] = m_resourceIndex;
        m_resourceIndex++;
    }

    template <typename T>
    uint32 getResourceIndex(){
        uint32 index = m_resourceMap[typeid(T)];
        return index;
    }

    // registerResource
    template <typename T>
    void registerResource(ResourceMetadata metadata){
        uint32 index = getResourceIndex<T>();
        T* resource = ((T*) dynamic_cast<T*>(m_resources.at(index)));
        resource->registerResource(metadata);
    }

    // getHandle
    template <typename T>
    auto getHandle(uint32 id){
        uint32 index = getResourceIndex<T>();
        T* resource = ((T*) dynamic_cast<T*>(m_resources.at(index)));
        return resource->getHandle(id);
    }

    // getData
    template <typename T, typename U>
    U* getData(Handle<U> handle){
        uint32 index = getResourceIndex<T>();
        T* resource = ((T*) dynamic_cast<T*>(m_resources.at(index)));
        return resource->getData(handle);
    }

private:
    std::vector<ResourceCache*> m_resources;
    rmap m_resourceMap;
    uint32 m_resourceIndex;
};
}