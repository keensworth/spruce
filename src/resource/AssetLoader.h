#pragma once

#include <fstream>
#include "ResourceTypes.h"
#include "external/flat_hash_map/flat_hash_map.hpp"

namespace spr{

typedef ska::flat_hash_map<uint32, std::string> PathMap;
typedef ska::flat_hash_map<uint32, std::string> NameMap;

class AssetLoader{
public:
    AssetLoader();
    ~AssetLoader();

    PathMap getPaths();
    NameMap getNames();

    friend class SprResourceManager;
private:
    PathMap m_paths;
    NameMap m_names;

    std::vector<ResourceMetadata> loadMetadata(
        std::vector<ResourceMetadata>& resourceMetadata,
        std::vector<uint32>& modelIds,
        std::vector<uint32>& textureIds);

};
}