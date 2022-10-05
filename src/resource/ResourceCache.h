#pragma once

//#//include "Registry.h"
//#include "../core/util/Container.h"
#include "../../data/asset_ids.h"
#include "../../external/flat_hash_map/flat_hash_map.hpp"
#include "../core/memory/Pool.h"
#include "ResourceLoader.h"

namespace spr {

class ResourceCache {
public:
    // register resource's metadata without loading data
    virtual void registerResource(ResourceMetadata metadata) = 0;

    // get resource's handle from id
    auto getHandle(uint32 id);

    // get resource's data from handle
    auto getData(auto handle);
};

template <typename T>
class TypedResourceCache : public ResourceCache {
public:
    TypedResourceCache(){}

    ~TypedResourceCache(){
        // clear hashmaps
        handles.clear();
        metadata.clear();
        // m_data destructor
    }

    // send metadata into hashmap with id
    void registerResource(ResourceMetadata metadata){
        m_metadata[metadata.resourceId] = metadata;
    }

    // get handle from hashmap with id
    Handle<T> getHandle(uint32 id){
        Handle<T> handle;

        // verify handle exists
        if (m_handles.count(id) > 0)
            handle = m_handles[id];
        
        // valid handle, up-to-date in pool
        if (handle.isValid() && m_pool.isValidHandle(handle))
            return handle;
        
        // handle isn't valid or isn't up-to-date in pool, load data
        T data = m_resourceLoader.loadFromMetadata<T>(m_metadata[id]);
        handle = m_data.insert(data);

        return handle;
    }

    // get data from pool with handle
    T* getData(Handle<T> handle){
        return m_data.get(handle);    
    }

private:
    // Resource metadata
    ska::flat_hash_map<uint32, ResourceMetadata> m_metadata;

    // Resource handles
    ska::flat_hash_map<uint32, Handle<T>> m_handles;

    // Resource data
    Pool<T> m_data;

    // Resource loader
    ResourceLoader m_resourceLoader;
};

}