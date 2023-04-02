#pragma once

#include <iostream>
#include <filesystem>
#include <fstream>

namespace spr::tools {

class AssetCreator{
public:
    AssetCreator();
    ~AssetCreator();

    void createTexture(std::string path);
private:
    std::string m_path;
    std::string m_name;
    std::string m_extension;

    void writeTextureFile(uint32_t bufferId, uint32_t components, uint32_t texId);
    void writeBufferFile(const unsigned char* data, uint32_t byteLength, uint32_t elementType, uint32_t componentType, uint32_t bufferId);
};

}