#pragma once

#include "ResourceCache.h"
#include "AssetLoader.h"
#include "../../external/flat_hash_map/flat_hash_map.hpp"



typedef ska::flat_hash_map<std::type_index, spr::ResourceCache*> rmap;

namespace spr {

class ModelCache : public TypedResourceCache<Model>{};
class MeshCache : public TypedResourceCache<Mesh>{};
class MaterialCache : public TypedResourceCache<Material>{};
class TextureCache : public TypedResourceCache<Texture>{};
class BufferCache : public TypedResourceCache<Buffer>{};

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager();

    // U := ResourceType
    template <typename U>
    Handle<U> getHandle(uint32 id){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        return typedCache->getHandle(id);
    }

    // U := ResourceType
    template <typename U>
    U* getData(Handle<U> handle){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        return typedCache->getData(handle);
    }
    
private:
    rmap m_resourceMap{
        {typeid(Model), new ModelCache},
        {typeid(Mesh), new MeshCache},
        {typeid(Material), new MaterialCache},
        {typeid(Texture), new TextureCache},
        {typeid(Buffer), new BufferCache},
    };

    void init();

    // U := ResourceType
    template <typename U>
    void registerResource(ResourceMetadata metadata){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        typedCache->registerResource(metadata);
    }
};
}