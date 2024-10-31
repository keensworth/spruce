// load from a path
#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include "ResourceTypes.h"
#include "data/asset_ids.h"
#include "external/mio/mio.h"

namespace spr {

typedef ska::flat_hash_map<uint32, ResourceMetadata> MetadataMap;
typedef ska::flat_hash_map<uint32, std::string> PathMap;

class ResourceLoader {
public:
    ResourceLoader();

    ~ResourceLoader(){}

    template <typename T>
    void loadFromMetadata(MetadataMap& metadataMap, ResourceMetadata& metadata, T& data);

    void checkMapping(uint32 id);
    void disable();

    void updateId(uint32 id){ m_id = id; }
    void updatePaths(PathMap pathMap){ m_pathMap = pathMap; }

private:
    uint32 m_id = 0;

    std::error_code m_error;
    mio::mmap_source m_mmap;
    uint32 m_mappedId = 0;

    PathMap m_pathMap;
};
}