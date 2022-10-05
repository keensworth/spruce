#include <iostream>
#include <string>
#include <filesystem>

//#define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "../../external/tinygltf/tiny_gltf.h"

namespace spr::tools{
class GLTFParser {
public:
    GLTFParser(){}
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

    void parse();
    void parseNode(const tinygltf::Node& node);
    void handleMesh(const tinygltf::Mesh& mesh);
    void handlePrimitive(const tinygltf::Primitive& primitive);
    void handleMaterial(const tinygltf::Material& material);
    void handleAccessor(const tinygltf::Accessor& accessor);
    void handleBufferView(const tinygltf::BufferView& bufferView, 
        int byteOffset, 
        int calcByteStride,
        int elementCount, 
        int elementType, 
        int componentType);
    void handleBuffer(
        const tinygltf::Buffer& buffer, 
        int byteOffset, 
        int byteLength);
    void handleBufferInterleaved(
        const tinygltf::Buffer& buffer, 
        int byteOffset, 
        int byteLength, 
        int byteStride,
        int bytesPerElement);
    void writeBufferFile(const unsigned char* data);
    void writeMeshFile();
    void writeMaterialFile();
    void writeModelFile();
    void writeTextureFile();
};
}