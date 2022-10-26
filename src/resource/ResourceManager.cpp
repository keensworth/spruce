#include "ResourceManager.h"

namespace spr{

ResourceManager::ResourceManager(){
    m_resourceCaches = std::vector<ResourceCache*>();
    m_resourceMap = rmap();
    m_resourceIndex = 0;

    // add resource types and store pointers to caches
    addResourceType<ModelCache>(new ModelCache);
    addResourceType<MeshCache>(new MeshCache);
    addResourceType<MaterialCache>(new MaterialCache);
    addResourceType<TextureCache>(new TextureCache);
    addResourceType<BufferCache>(new BufferCache);
}

ResourceManager::~ResourceManager(){
    // delete model cache
    uint32 index = getResourceCacheIndex<ModelCache>();
    ModelCache* modelCache = ((ModelCache*) dynamic_cast<ModelCache*>(m_resourceCaches.at(index)));
    delete modelCache;

    // delete mesh cache
    index = getResourceCacheIndex<MeshCache>();
    MeshCache* meshCache = ((MeshCache*) dynamic_cast<MeshCache*>(m_resourceCaches.at(index)));
    delete meshCache;

    // delete material cache
    index = getResourceCacheIndex<MaterialCache>();
    MaterialCache* materialCache = ((MaterialCache*) dynamic_cast<MaterialCache*>(m_resourceCaches.at(index)));
    delete materialCache;

    // delete texture cache
    index = getResourceCacheIndex<TextureCache>();
    TextureCache* textureCache = ((TextureCache*) dynamic_cast<TextureCache*>(m_resourceCaches.at(index)));
    delete textureCache;

    // delete buffer cache
    index = getResourceCacheIndex<BufferCache>();
    BufferCache* bufferCache = ((BufferCache*) dynamic_cast<BufferCache*>(m_resourceCaches.at(index)));
    delete bufferCache;
}

void ResourceManager::init(){
    // load resource metadata from asset_manifest.json
    AssetLoader assetLoader;
    std::vector<ResourceMetadata> resourceMetadata = assetLoader.loadMetadata();

    // register resources with metadata
    for (auto & metadata : resourceMetadata){
        switch(metadata.resourceType) {
            case SPR_MESH :
                registerResource<MeshCache>(metadata);
                break;
            case SPR_MODEL :
                registerResource<ModelCache>(metadata);
                break;
            case SPR_AUDIO :
                //registerResource<AudioCache>(metadata);
                break;
            case SPR_SHADER :
                //registerResource<ShaderCache>(metadata);
                break;
            case SPR_BUFFER :
                registerResource<BufferCache>(metadata);
                break;
            case SPR_TEXTURE :
                registerResource<TextureCache>(metadata);
                break;
            case SPR_MATERIAL :
                registerResource<MaterialCache>(metadata);
                break;
            default:
                // log error
                break;
        }
    }
}

}