#include "AssetLoader.h"
#include "../debug/SprLog.h"

namespace spr{

std::vector<ResourceMetadata> AssetLoader::loadMetadata(
        std::vector<ResourceMetadata>& resourceMetadata,
        std::vector<uint32>& modelIds,
        std::vector<uint32>& textureIds){

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
        modelIds.push_back(metadata.resourceId);
    }

    // load non-subresource textures
    for (const auto& texture : manifest["nonSubresourceTextures"]) {
        ResourceMetadata metadata;
        metadata.sizeBytes    = texture["sizeBytes"];
        metadata.resourceId   = texture["id"];
        metadata.name         = texture["name"];
        metadata.resourceType = ResourceTypes::stringToType(texture["type"]);

        resourceMetadata.push_back(metadata);
        textureIds.push_back(metadata.resourceId);
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

    f.close();

    return resourceMetadata;
}

}