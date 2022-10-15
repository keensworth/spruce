#pragma once

#include <iostream>
#include <filesystem>
#include "../../external/tinygltf/tiny_gltf.h"

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
    int m_id = 0;

    void parse();
    void parseNode(const tinygltf::Node& node, std::vector<int> &meshIds);
    void handleMesh(const tinygltf::Mesh& mesh, std::vector<int> &meshIds);
    int handlePrimitive(const tinygltf::Primitive& primitive);
    int handleMaterial(const tinygltf::Material& material);
    int handleTexture(const tinygltf::TextureInfo& texInfo);
    int handleTexture(const tinygltf::NormalTextureInfo& texInfo);
    int handleTexture(const tinygltf::OcclusionTextureInfo& texInfo);
    int handleTexture(const tinygltf::Texture& tex);
    int handleAccessor(const tinygltf::Accessor& accessor);
    void handleBufferView(const tinygltf::BufferView& bufferView, 
        int byteOffset, 
        int calcByteStride,
        int elementCount, 
        int elementType, 
        int componentType,
        int bufferId);
    void handleBuffer(
        const tinygltf::Buffer& buffer, 
        int byteOffset, 
        int byteLength, 
        int elementType, 
        int componentType,
        int bufferId);
    void handleBufferInterleaved(
        const tinygltf::Buffer& buffer, 
        int byteOffset, 
        int byteLength, 
        int byteStride,
        int bytesPerElement, 
        int elementType, 
        int componentType,
        int bufferId);
    void writeBufferFile(const unsigned char* data, int byteLength, int elementType, int componentType, int bufferId);
    void writeMeshFile(
        int materialId, 
        int indexBufferId, 
        int posBufferId, 
        int normalBufferId, 
        int colorBufferId, 
        int tangentBufferId, 
        std::vector<int> texCoordBufferIds,
        int meshId);
    void writeMaterialFile(
        int materialFlags, 
        const tinygltf::Material& material,
        std::vector<int> texIds,
        int materialId);
    void writeModelFile(int meshCount, std::vector<int> meshIds, int modelId);
    void writeTextureFile(int bufferId, int imageType, int texId);
};
}