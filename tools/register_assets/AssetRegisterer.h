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
#include "../../src/resource/ResourceTypes.h"
#include "../../external/json/json.hpp"
#include "../../external/flat_hash_map/flat_hash_map.hpp"

namespace spr::tools{

class AssetRegisterer{
public:
    AssetRegisterer(){}
    ~AssetRegisterer(){}

    void registerDirectory(std::string dir);
    int loadModel(std::string path);
    int loadMesh(std::string path);
    int loadMaterial(std::string path);
    int loadTexture(std::string path);
    int loadBuffer(std::string path);
    void writeHeader();
    void writeManifest(int totalBytes);
private:
    // Filename <-> ResourceId map 
    ska::flat_hash_map<std::string, ResourceMetadata> m_metadataMap;
    ska::flat_hash_map<std::string, ResourceMetadata> m_modelMetadataMap;

    uint32 m_id = 1;
};

}