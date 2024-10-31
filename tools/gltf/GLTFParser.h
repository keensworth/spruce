#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include "../../external/tinygltf/tiny_gltf.h"
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/gtc/matrix_inverse.hpp>
#include "Resources.h"

typedef std::unordered_map<uint32_t, uint32_t> IdMap;

namespace spr::tools{

enum DataRegion {
    SPR_DR_INDEX = 0,
    SPR_DR_POSITION = 1,
    SPR_DR_ATTRIBUTE = 2,
    SPR_DR_TEXTURE = 3
};

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

struct OffsetSpan {
    uint32_t sizeBytes = 0;
    uint32_t offset = 0;
};

class GLTFParser {
public:
    GLTFParser();
    ~GLTFParser(){}

    void parseJson(std::string path);
    void parseBinary(std::string path);
private:
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    std::string m_path;
    std::string m_name;
    std::string m_extension;
    uint32_t m_id = 0;
    IdMap m_sourceBuffIdMap;
    IdMap m_sourceTexIdMap;

    std::ofstream m_outputStream;
    std::ofstream m_modelStream;
    std::ofstream m_meshStream;
    std::ofstream m_materialStream;
    std::ofstream m_textureStream;
    std::ofstream m_indexDataStream;
    std::ofstream m_positionDataStream;
    std::ofstream m_attributeDataStream;
    std::ofstream m_textureDataStream;

    std::ifstream m_modelStreamI;
    std::ifstream m_meshStreamI;
    std::ifstream m_materialStreamI;
    std::ifstream m_textureStreamI;
    std::ifstream m_indexDataStreamI;
    std::ifstream m_positionDataStreamI;
    std::ifstream m_attributeDataStreamI;
    std::ifstream m_textureDataStreamI;

    uint32 m_meshIndex = 0;
    uint32 m_materialIndex = 0;
    uint32 m_textureIndex = 0;

    uint32 m_meshCount = 0;
    uint32 m_materialCount = 0;
    uint32 m_textureCount = 0;

    uint32 m_indicesOffset = 0;
    uint32 m_positionOffset = 0;
    uint32 m_attributesOffset = 0;
    uint32 m_textureDataOffset = 0;

    void parse();
    void init();
    void consolidate();
    void cleanup();
    void parseNode(const tinygltf::Node& node, std::vector<uint32_t> &meshIds, glm::mat4& transform);
    void handleMesh(const tinygltf::Mesh& mesh, std::vector<uint32_t> &meshIds, glm::mat4& transform);
    uint32_t handlePrimitive(const tinygltf::Primitive& primitive, glm::mat4& transform);
    OffsetSpan interleaveVertexAttributes(
        uint32_t vertexCount,
        std::vector<uint8_t>& normalBuffer,
        std::vector<uint8_t>& tangentBuffer,
        std::vector<uint8_t>& texCoordBuffer,
        std::vector<uint8_t>& colorBuffer,
        glm::mat4& transform,
        DataRegion region);
    uint32 handleMaterial(const tinygltf::Material& material, uint32_t& outMaterialFlags);
    uint32 handleTexture(const tinygltf::TextureInfo& texInfo, BufferData dataType);
    uint32 handleTexture(const tinygltf::NormalTextureInfo& texInfo);
    uint32 handleTexture(const tinygltf::OcclusionTextureInfo& texInfo);
    uint32 handleTexture(const tinygltf::Texture& tex, BufferData dataType);
    OffsetSpan handleAccessor(const tinygltf::Accessor& accessor, 
        std::vector<uint8_t>& out, 
        bool writeToFile, 
        BufferData dataType, 
        glm::mat4& transform,
        DataRegion region);
    OffsetSpan handleBufferView(const tinygltf::BufferView& bufferView, 
        std::string association,
        uint32_t byteOffset, 
        uint32_t calcByteStride,
        uint32_t elementCount, 
        uint32_t elementType, 
        uint32_t componentType,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        glm::mat4& transform,
        DataRegion region);
    OffsetSpan handleBuffer(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t bytesPerElement,
        uint32_t elementCount,
        uint32_t elementType, 
        uint32_t componentType,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        glm::mat4& transform,
        DataRegion region);
    OffsetSpan handleTextureBuffer(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t bytesPerElement,
        uint32_t elementCount,
        uint32_t elementType, 
        uint32_t componentType,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        uint32_t width,
        uint32_t height,
        uint32_t components);
    OffsetSpan handleMIMEImageBuffer(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t bytesPerElement,
        uint32_t elementCount,
        uint32_t elementType, 
        uint32_t componentType,
        BufferData dataType);
    void createMip(
        unsigned char* in, 
        uint32_t inSizeBytes, 
        uint32_t inExtent, 
        unsigned char* mipOut, 
        uint32_t outSizeBytes, 
        uint32_t outExtent);
    void compressImageData(
        unsigned char* data,
        uint32_t dataSize,
        unsigned char** outData,
        uint32_t& outDataSize,
        BufferData dataType,
        uint32_t width,
        uint32_t height,
        uint32_t components);
    OffsetSpan handleBufferInterleaved(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t byteStride,
        uint32_t bytesPerElement, 
        uint32_t elementType, 
        uint32_t componentType,
        std::vector<uint8_t>& out,
        bool writeToFile,
        DataRegion region);
    OffsetSpan writeBufferFile(const unsigned char* data, uint32_t byteLength, DataRegion dataRegion);
    uint32 writeTextureFile(TextureLayout& texture);
    uint32 writeMaterialFile(MaterialLayout& material);
    uint32 writeMeshFile(MeshLayout& mesh);


    void vectorToMat4(const std::vector<double>& src, glm::mat4& dst);
    void vectorToQuat(const std::vector<double>& src, glm::quat& dst);
    void vectorToVec3(const std::vector<double>& src, glm::vec3& dst);
};
}