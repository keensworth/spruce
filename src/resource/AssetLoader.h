#pragma once

#include <fstream>
#include "ResourceTypes.h"
#include "../../external/json/json.hpp"

namespace spr{
class AssetLoader{
public:
    AssetLoader(){}
    ~AssetLoader(){}

    friend class SprResourceManager;
private:

    std::vector<ResourceMetadata> loadMetadata();

};
}