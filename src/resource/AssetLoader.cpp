#include "AssetLoader.h"
#include "../debug/SprLog.h"

namespace spr{

std::vector<ResourceMetadata> AssetLoader::loadMetadata(){
    std::vector<ResourceMetadata> resourceMetadata;

    std::ifstream f("../data/asset_manifest.json");
    nlohmann::json manifest = nlohmann::json::parse(f);

    // load models
    for (const auto& model : manifest["models"]) {
        ResourceMetadata metadata;
        metadata.sizeBytes    = model["sizeBytes"];
        metadata.resourceId   = model["id"];
        metadata.name         = model["name"];
        metadata.resourceType = ResourceTypes::stringToType(model["type"]);

        resourceMetadata.push_back(metadata);
    }

    // load non-subresource textures
    for (const auto& subresource : manifest["nonSubresourceTextures"]) {
        ResourceMetadata metadata;
        metadata.sizeBytes    = subresource["sizeBytes"];
        metadata.resourceId   = subresource["id"];
        metadata.name         = subresource["name"];
        metadata.resourceType = ResourceTypes::stringToType(subresource["type"]);

        resourceMetadata.push_back(metadata);
    }

    // load subresources
    for (const auto& subresource : manifest["subresources"]) {
        ResourceMetadata metadata;
        metadata.sizeBytes    = subresource["sizeBytes"];
        metadata.resourceId   = subresource["id"];
        metadata.name         = subresource["name"];
        metadata.resourceType = ResourceTypes::stringToType(subresource["type"]);

        resourceMetadata.push_back(metadata);
    }

    return resourceMetadata;
}

}