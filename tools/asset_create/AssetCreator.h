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
    void createCubemapTexture(std::string facePaths[6]);
private:
    enum BufferData {
        SPR_NONE = 0,
        SPR_POSITION = 1,
        SPR_INDICES = 2,
        SPR_NORMALS = 3,
        SPR_COLOR = 4,
        SPR_TANGENTS = 5,
        SPR_UV = 6,
        SPR_TEXTURE_COLOR = 7,
        SPR_TEXTURE_NORMAL = 8,
        SPR_TEXTURE_OTHER = 9,
    };

    std::string m_path;
    std::string m_name;
    std::string m_extension;

    void writeTextureFile(const unsigned char* data, uint32_t byteLength, uint32_t height, uint32_t width, uint32_t components);
    void compressImageData(
        unsigned char* data,
        uint32_t dataSize,
        unsigned char** outData,
        uint32_t& outDataSize,
        BufferData dataType,
        uint32_t width,
        uint32_t height,
        uint32_t components);
    void createMip(
        unsigned char* in, 
        uint32_t inSizeBytes, 
        uint32_t inExtent, 
        unsigned char* mipOut, 
        uint32_t outSizeBytes, 
        uint32_t outExtent);
    void layerImageData(
        unsigned char* data[6],
        uint32_t dataSize,
        unsigned char** outData,
        uint32_t& outDataSize,
        BufferData dataType,
        uint32_t width,
        uint32_t height,
        uint32_t components);
};

}