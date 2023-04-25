#include "SprResourceManager.h"
#include "../debug/SprLog.h"

namespace spr{

SprResourceManager::SprResourceManager(){
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
    std::vector<ResourceMetadata> resourceMetadata;

    AssetLoader assetLoader;
    assetLoader.loadMetadata(resourceMetadata, m_modelIds, m_textureIds);

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

std::vector<uint32>& SprResourceManager::getModelIds(){
    return m_modelIds;
}
std::vector<uint32>& SprResourceManager::getTextureIds(){
    return m_textureIds;
}

}