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

    void writeTextureFile(uint32_t bufferId, uint32_t height, uint32_t width, uint32_t components, uint32_t texId);
    void writeBufferFile(const unsigned char* data, uint32_t byteLength, uint32_t elementType, uint32_t componentType, uint32_t bufferId);
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
};

}