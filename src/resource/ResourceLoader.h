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

struct ResourceIds {
    uint32 id1 = 0;
    uint32 id2 = 0;
    uint32 id3 = 0;
    uint32 id4 = 0;
    uint32 id5 = 0;
    uint32 id6 = 0;
    uint32 id7 = 0;
    uint32 id8 = 0;
};

struct ResourceIdsList {
    std::vector<uint32> ids;
};

typedef ska::flat_hash_map<uint32, ResourceMetadata> MetadataMap;
typedef ska::flat_hash_map<uint32, std::string> PathMap;
typedef ska::flat_hash_map<uint32, ResourceIds> IdMap;
typedef ska::flat_hash_map<uint32, ResourceIdsList> IdListMap;

class ResourceLoader {
public:
    ResourceLoader();

    ~ResourceLoader(){}

    template <typename T>
    void loadFromMetadata(MetadataMap& metadataMap, ResourceMetadata& metadata, T& data);

    bool checkMapping(uint32 id);
    void disable();

    void updateId(uint32 id){ m_id = id; }
    void updatePaths(PathMap pathMap){ m_pathMap = pathMap; }

private:
    uint32 m_id = 0;
    uint32 m_texCount = 0;

    std::error_code m_error;
    mio::mmap_source m_mmap;
    uint32 m_mappedId = 0;

    PathMap m_pathMap;
    IdMap m_idMap;
    IdListMap m_idListMap;
};
}