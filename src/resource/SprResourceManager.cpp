#include "SprResourceManager.h"

namespace spr{

SprResourceManager::SprResourceManager(){
    // m_resourceCaches = std::vector<ResourceCache*>();
    // m_resourceMap = rmap();
    // m_resourceIndex = 0;

    // // add resource types and store pointers to caches
    // addResourceType<Model>(new ModelCache);
    // addResourceType<Mesh>(new MeshCache);
    // addResourceType<Material>(new MaterialCache);
    // addResourceType<Texture>(new TextureCache);
    // addResourceType<Buffer>(new BufferCache);
    init();
}

SprResourceManager::~SprResourceManager(){
    // delete model cache
    ModelCache* modelCache = ((ModelCache*) m_resourceMap[typeid(Model)]);
    delete modelCache;

    // delete mesh cache
    MeshCache* meshCache = ((MeshCache*) m_resourceMap[typeid(Mesh)]);
    delete meshCache;

    // delete material cache
    MaterialCache* materialCache = ((MaterialCache*) m_resourceMap[typeid(Material)]);
    delete materialCache;

    // delete texture cache
    TextureCache* textureCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);
    delete textureCache;

    // delete buffer cache
    BufferCache* bufferCache = ((BufferCache*) m_resourceMap[typeid(Buffer)]);
    delete bufferCache;
}

void SprResourceManager::init(){
    // load resource metadata from asset_manifest.json
    AssetLoader assetLoader;
    std::vector<ResourceMetadata> resourceMetadata = assetLoader.loadMetadata();

    // register resources with metadata
    for (auto & metadata : resourceMetadata){
        switch(metadata.resourceType) {
            case SPR_MESH :
                registerResource<Mesh>(metadata);
                break;
            case SPR_MODEL :
                registerResource<Model>(metadata);
                break;
            case SPR_AUDIO :
                //registerResource<AudioCache>(metadata);
                break;
            case SPR_SHADER :
                //registerResource<ShaderCache>(metadata);
                break;
            case SPR_BUFFER :
                registerResource<Buffer>(metadata);
                break;
            case SPR_TEXTURE :
                registerResource<Texture>(metadata);
                break;
            case SPR_MATERIAL :
                registerResource<Material>(metadata);
                break;
            default:
                SprLog::warn("[SprResourceManager] ResourceType not recognized");
                break;
        }
    }
}

}