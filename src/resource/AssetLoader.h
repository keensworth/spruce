#pragma once

#include "ResourceTypes.h"
#include <fstream>
#include "../../external/json/json.hpp"

namespace spr{
class AssetLoader{
public:
    AssetLoader(){}
    ~AssetLoader(){}

    friend class ResourceManager;
private:

    std::vector<ResourceMetadata> loadMetadata();

};
}