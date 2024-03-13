#pragma once

#include "ResourceCache.h"
#include "AssetLoader.h"
#include "../../external/flat_hash_map/flat_hash_map.hpp"


namespace spr {

typedef ska::flat_hash_map<std::type_index, spr::ResourceCache*> CacheMap;

class ModelCache : public TypedResourceCache<Model>{};
class MeshCache : public TypedResourceCache<Mesh>{};
class MaterialCache : public TypedResourceCache<Material>{};
class TextureCache : public TypedResourceCache<Texture>{};
class BufferCache : public TypedResourceCache<Buffer>{};

class SprResourceManager {
public:
    SprResourceManager();
    ~SprResourceManager();

    uint32 getSize(){
        return m_resourceMap[typeid(Buffer)]->getSize();
    }

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

    // U := ResourceType
    template <typename U>
    U* getData(uint32 id){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        auto handle = typedCache->getHandle(id);
        return typedCache->getData(handle);
    }

    // U := ResourceType
    template <typename U>
    void deleteData(uint32 id){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        auto handle = typedCache->getHandle(id);
        return typedCache->deleteData(handle);
    }

    // U := ResourceType
    template <typename U>
    void deleteData(Handle<U> handle){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        return typedCache->deleteData(handle);
    }

    void destroyBuffers(){
        m_resourceMap[typeid(Buffer)]->destroy();
    }

    std::vector<uint32>& getModelIds();
    std::vector<uint32>& getTextureIds();
    
private:
    CacheMap m_resourceMap{
        {typeid(Model), new ModelCache},
        {typeid(Mesh), new MeshCache},
        {typeid(Material), new MaterialCache},
        {typeid(Texture), new TextureCache},
        {typeid(Buffer), new BufferCache},
    };

    std::vector<uint32> m_modelIds;
    std::vector<uint32> m_textureIds;

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