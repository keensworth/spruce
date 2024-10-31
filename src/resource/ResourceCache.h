#pragma once

#include "external/flat_hash_map/flat_hash_map.hpp"
#include "core/memory/Pool.h"
#include "ResourceLoader.h"

namespace spr {

class ResourceCache {
public:
    virtual ~ResourceCache() = default;

    // get resource's handle from id
    auto getHandle(uint32 id);

    // get resource's data from handle
    auto getData(auto handle);

    // delete resource's data from handle
    void deleteData(auto handle);

    virtual uint32 getSize() = 0;

    virtual void destroy() = 0;
};

template <typename T>
class TypedResourceCache : public ResourceCache {
public:
    TypedResourceCache() : m_data(4096){}

    ~TypedResourceCache(){
        // clear hashmaps
        m_handles.clear();
    }

    // get handle from hashmap with id
    inline Handle<T> getHandle(uint32 id, ResourceLoader& resourceLoader, MetadataMap& metadataMap){
        Handle<T> handle;

        // verify handle exists
        if (m_handles.count(id) > 0)
            handle = m_handles[id];
        
        // valid handle, up-to-date in pool
        if (handle.isValid() && m_data.isValidHandle(handle))
            return handle;
        
        // handle isn't valid or isn't up-to-date in pool, 
        // load data and update
        T data;
        resourceLoader.loadFromMetadata<T>(metadataMap, metadataMap[id], data);
        handle = m_data.insert(data);
        m_handles[id] = handle;

        return handle;
    }

    // get data from pool with handle
    inline T* getData(Handle<T> handle){
        return m_data.get(handle);    
    }

    inline void deleteData(Handle<T> handle){
        if (!handle.isValid() || !m_data.isValidHandle(handle)){
            return;
        }
        m_data.remove(handle);
    }
    
    void destroy(){
        m_data.destroy();
    }

    uint32 getSize(){
        return m_data.getSize();
    }

    friend class SprResourceManager;

private:
    ska::flat_hash_map<uint32, Handle<T>> m_handles;
    Pool<T> m_data;
};

}