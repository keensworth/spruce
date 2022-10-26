#include "AssetLoader.h"

namespace spr{

std::vector<ResourceMetadata> AssetLoader::loadMetadata(){
    std::vector<ResourceMetadata> resourceMetadata;

    std::ifstream f("../data/asset_manifest.json");
    nlohmann::json manifest = nlohmann::json::parse(f);

    // load models
    for (const nlohmann::json& model : manifest["models"].items()) {
        ResourceMetadata metadata;
        metadata.sizeBytes = model["sizeBytes"];
        metadata.resourceId = model["id"];
        metadata.name = model["name"];
        metadata.resourceType = ResourceTypes::stringToType(model["type"]);

        resourceMetadata.push_back(metadata);
    }

    // load subresources
    for (const nlohmann::json& subresource : manifest["subresources"].items()) {
        ResourceMetadata metadata;
        metadata.sizeBytes = subresource["sizeBytes"];
        metadata.resourceId = subresource["id"];
        metadata.name = subresource["name"];
        metadata.resourceType = ResourceTypes::stringToType(subresource["type"]);
        resourceMetadata.push_back(metadata);
    }

    return resourceMetadata;
}

}