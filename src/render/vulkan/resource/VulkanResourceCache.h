#pragma once

#include "ResourceTypes.h"
#include "../../../../external/flat_hash_map/flat_hash_map.hpp"
#include "../../../core/memory/Pool.h"

namespace spr::gfx {

class VulkanResourceCache {
public:
    virtual ~VulkanResourceCache();
    // get resource's data from handle
    auto get(auto handle);

    // insert resource data, return handle
    auto insert(auto data);

    // insert resource data, return handle
    void remove(auto handle);
};

template <typename T>
class TypedResourceCache : public VulkanResourceCache {
public:
    TypedResourceCache(){}

    ~TypedResourceCache(){}

    // get data from pool with handle
    T* get(Handle<T> handle){
        return m_data.get(handle);    
    }

    // insert data into pool
    Handle<T> insert(T data){
        return m_data.insert(data);
    }

    // invalidate handle, add to freelist
    void remove(Handle<T> handle){
        m_data.remove(handle);
        return;
    }

    friend class VulkanResourceManager;

private:
    // Resource data
    Pool<T> m_data;
};

}