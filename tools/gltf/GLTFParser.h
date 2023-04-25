#pragma once

#include <iostream>
#include <filesystem>
#include <unordered_map>
#include "../../external/tinygltf/tiny_gltf.h"

typedef std::unordered_map<uint32_t, uint32_t> IdMap;

namespace spr::tools{
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
    IdMap m_sourceIdMap;

    void parse();
    void parseNode(const tinygltf::Node& node, std::vector<uint32_t> &meshIds);

    void handleMesh(const tinygltf::Mesh& mesh, std::vector<uint32_t> &meshIds);
    uint32_t handlePrimitive(const tinygltf::Primitive& primitive);
    uint32_t interleaveVertexAttributes(
        uint32_t vertexCount,
        std::vector<uint8_t>& normalBuffer,
        std::vector<uint8_t>& tangentBuffer,
        std::vector<uint8_t>& texCoordBuffer,
        std::vector<uint8_t>& colorBuffer);
    uint32_t handleMaterial(const tinygltf::Material& material);
    uint32_t handleTexture(const tinygltf::TextureInfo& texInfo);
    uint32_t handleTexture(const tinygltf::NormalTextureInfo& texInfo);
    uint32_t handleTexture(const tinygltf::OcclusionTextureInfo& texInfo);
    uint32_t handleTexture(const tinygltf::Texture& tex);
    uint32_t handleAccessor(const tinygltf::Accessor& accessor, std::vector<uint8_t>& out, bool writeToFile);
    void handleBufferView(const tinygltf::BufferView& bufferView, 
        uint32_t byteOffset, 
        uint32_t calcByteStride,
        uint32_t elementCount, 
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId,
        std::vector<uint8_t>& out,
        bool writeToFile);
    void handleBuffer(
        const tinygltf::Buffer& buffer, 
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId,
        std::vector<uint8_t>& out,
        bool writeToFile);
    void handleBufferInterleaved(
        const tinygltf::Buffer& buffer, 
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t byteStride,
        uint32_t bytesPerElement, 
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId,
        std::vector<uint8_t>& out,
        bool writeToFile);
    void writeBufferFile(
        const unsigned char* data, 
        uint32_t byteLength, 
        uint32_t elementType, 
        uint32_t componentType, 
        uint32_t bufferId);
    void writeMeshFile(
        uint32_t materialId, 
        uint32_t indexBufferId, 
        uint32_t posBufferId,  
        uint32_t attributesBufferId,
        uint32_t meshId);
    void writeMaterialFile(
        uint32_t materialFlags, 
        const tinygltf::Material& material,
        std::vector<uint32_t> texIds,
        uint32_t materialId);
    void writeModelFile(uint32_t meshCount, std::vector<uint32_t> meshIds, uint32_t modelId);
    void writeTextureFile(uint32_t bufferId, uint32_t height, uint32_t width, uint32_t imageType, uint32_t texId);
};
}