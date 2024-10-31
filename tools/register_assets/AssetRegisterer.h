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

class AssetRegisterer{
public:
    AssetRegisterer(){}
    ~AssetRegisterer(){}

    void registerDirectory(std::string dir);
    int loadModel(std::string path, mio::mmap_source& file);
    int loadMesh(std::string path, ResourceMetadata& modelData, mio::mmap_source& file, ModelHeader& model, uint32 index);
    int loadMaterial(std::string path, ResourceMetadata& modelData, mio::mmap_source& file, ModelHeader& model, MeshLayout& mesh);
    int loadTexture(std::string path, ResourceMetadata& modelData, bool subresource, mio::mmap_source& file, ModelHeader& model, uint32 index);
    int loadBuffer(std::string path, ResourceMetadata& modelData, mio::mmap_source& file, ModelHeader& model, uint32 offset, uint32 length);
    void writeHeader();
    void writeManifest(int totalBytes);

    static std::string typeToString(ResourceType resourceType){
        return resourceTypeStrings[resourceType];
    }
private:
    // Filename <-> ResourceId map 
    ska::flat_hash_map<std::string, ResourceMetadata> m_metadataMap;
    ska::flat_hash_map<std::string, ResourceMetadata> m_modelMetadataMap;
    ska::flat_hash_map<std::string, ResourceMetadata> m_nonSubresourceTextureMap;

    ska::flat_hash_map<uint32_t, uint32> m_texturePresenceMap;
    uint32 m_id = 3;

    ska::flat_hash_map<std::string, uint32> m_headerLowerNameMap;
};

}