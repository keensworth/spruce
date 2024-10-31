#include "SprResourceManager.h"

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
        registerResource(metadata);
        m_id = glm::max(metadata.resourceId, m_id);
    }

    m_names = assetLoader.getNames();
    m_resourceLoader.updateId(m_id);
    m_resourceLoader.updatePaths(assetLoader.getPaths());
}



}