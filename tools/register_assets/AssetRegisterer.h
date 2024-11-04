#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cstddef> 
#include <locale>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <sstream>
#include "../../external/json/json.hpp"
#include "../../external/flat_hash_map/flat_hash_map.hpp"
#include "../../external/mio/mio.h"
#include "../gltf/Resources.h"

namespace spr::tools{

static std::vector<std::string> resourceTypeStrings{
    "SPR_NONE",
    "SPR_MESH",
    "SPR_MODEL",
    "SPR_AUDIO",
    "SPR_SHADER",
    "SPR_BUFFER",
    "SPR_TEXTURE",
    "SPR_MATERIAL",
};

enum ResourceType : uint32 {
    SPR_NONE,
    SPR_MESH,
    SPR_MODEL,
    SPR_AUDIO,
    SPR_SHADER,
    SPR_BUFFER,
    SPR_TEXTURE,
    SPR_MATERIAL
};


struct ResourceMetadata {
    std::string name;
    ResourceType resourceType = SPR_NONE;     
    uint32 resourceId = 0;  // resource-unique id (enum value)
    uint32 parentId = 0;
    uint32 sizeTotal = 0;
    uint32 byteOffset = 0;
    uint32 byteLength = 0;
    uint32 index = 0;
    uint32 sub = 1;
};

struct LoadResult {
    uint32 sizeBytes = 0;
    uint32 resultId = 0;
};

class AssetRegisterer{
public:
    AssetRegisterer(){}
    ~AssetRegisterer(){}

    void registerDirectory(std::string dir);
    LoadResult loadModel();
    LoadResult loadMesh(ResourceMetadata& modelData, uint32 index);
    LoadResult loadMaterial(ResourceMetadata& modelData, MeshLayout& mesh);
    LoadResult loadTexture(ResourceMetadata& modelData, uint32 index);
    LoadResult loadDedicatedTexture(ResourceMetadata& modelData);
    LoadResult loadBuffer(ResourceMetadata& modelData, uint32 offset, uint32 length);
    void checkinModel();
    void checkinDedicatedTexture(ResourceMetadata& modelData);
    void writeHeader();
    void writeManifest(int totalBytes);

    static std::string typeToString(ResourceType resourceType){
        return resourceTypeStrings[resourceType];
    }
private:
    mio::mmap_sink rw_mmap;
    std::error_code m_error;

    // Filename <-> ResourceId map 
    ska::flat_hash_map<std::string, ResourceMetadata> m_metadataMap;
    ska::flat_hash_map<std::string, ResourceMetadata> m_modelMetadataMap;
    ska::flat_hash_map<std::string, ResourceMetadata> m_nonSubresourceTextureMap;

    ska::flat_hash_map<uint32_t, uint32> m_texturePresenceMap;
    ska::flat_hash_map<uint32_t, uint32> m_materialPresenceMap;
    uint32 m_id = 3;

    ska::flat_hash_map<std::string, uint32> m_headerLowerNameMap;
};

}