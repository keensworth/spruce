#include "AssetLoader.h"
#include "debug/SprLog.h"
#include "external/json/json.hpp"


namespace spr{

AssetLoader::AssetLoader(){}
    
AssetLoader::~AssetLoader(){
    m_paths.clear();
    m_names.clear();
}

PathMap AssetLoader::getPaths(){ return m_paths; }
NameMap AssetLoader::getNames(){ return m_names; }

std::vector<ResourceMetadata> AssetLoader::loadMetadata(
        std::vector<ResourceMetadata>& resourceMetadata,
        std::vector<uint32>& modelIds,
        std::vector<uint32>& textureIds){

    std::ifstream f("../data/asset_manifest.json");
    nlohmann::json manifest = nlohmann::json::parse(f);

    // load models
    for (const auto& model : manifest["models"]) {
        std::string name = model["name"];

        ResourceMetadata metadata;
        metadata.sizeTotal    = model["sizeTotal"];
        metadata.resourceId   = model["id"];
        metadata.parentId     = model["parentId"];
        
        metadata.resourceType = ResourceTypes::stringToType(model["type"]);
        metadata.sub = 0;

        resourceMetadata.push_back(metadata);
        modelIds.push_back(metadata.resourceId);
        m_paths[metadata.parentId] = ResourceTypes::path(metadata.resourceType, name, metadata.sub);
        m_names[metadata.parentId] = name;
    }

    // load non-subresource textures
    for (const auto& texture : manifest["nonSubresourceTextures"]) {
        std::string name = texture["name"];

        ResourceMetadata metadata;
        metadata.sizeTotal    = texture["sizeTotal"];
        metadata.resourceId   = texture["id"];
        metadata.parentId     = texture["parentId"];
        metadata.resourceType = ResourceTypes::stringToType(texture["type"]);
        metadata.sub = 0;

        resourceMetadata.push_back(metadata);
        textureIds.push_back(metadata.resourceId);
        m_paths[metadata.parentId] = ResourceTypes::path(metadata.resourceType, name, metadata.sub);
        m_names[metadata.parentId] = name;
    }

    f.close();

    return resourceMetadata;
}

}