#pragma once

#include "ResourceCache.h"
#include "AssetLoader.h"
#include "external/flat_hash_map/flat_hash_map.hpp"


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


    // U := ResourceType
    template <typename U>
    Handle<U> getHandle(uint32 id){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        return typedCache->getHandle(id, m_resourceLoader, m_metadata);
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
        auto handle = typedCache->getHandle(id, m_resourceLoader, m_metadata);
        return typedCache->getData(handle);
    }

    // U := ResourceType
    template <typename U>
    void deleteData(uint32 id){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        auto handle = typedCache->getHandle(id, m_resourceLoader, m_metadata);
        return typedCache->deleteData(handle);
    }

    // U := ResourceType
    template <typename U>
    void deleteData(Handle<U> handle){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        return typedCache->deleteData(handle);
    }

    void getName(uint32 id, std::string& out){
        if (m_names.count(id) > 0)
            out = m_names[id];
        else
            out = "no-name";
    }

    void destroyBuffers(){
        m_resourceMap[typeid(Buffer)]->destroy();
    }

    void disableLoader(){
        m_resourceLoader.disable();
    }

    uint32 getSize(){
        return m_resourceMap[typeid(Buffer)]->getSize();
    }

    std::vector<uint32>& getModelIds(){
        return m_modelIds;
    }

    std::vector<uint32>& getTextureIds(){
        return m_textureIds;
    }
    
private:
    NameMap m_names;
    MetadataMap m_metadata;
    ResourceLoader m_resourceLoader;

    CacheMap m_resourceMap {
        {typeid(Model), new ModelCache},
        {typeid(Mesh), new MeshCache},
        {typeid(Material), new MaterialCache},
        {typeid(Texture), new TextureCache},
        {typeid(Buffer), new BufferCache},
    };

    std::vector<uint32> m_modelIds;
    std::vector<uint32> m_textureIds;
    uint32 m_id = 0;

    void init();

    void registerResource(ResourceMetadata& metadata){
        m_metadata[metadata.resourceId] = metadata;
    }
};
}