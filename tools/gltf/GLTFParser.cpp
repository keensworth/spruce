//#define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
//#include "../../external/tinygltf/tiny_gltf.h"

//using namespace tinygltf;

#include <fstream>
#include <glm/glm.hpp>
#include "GLTFParser.h"

namespace spr::tools{

void GLTFParser::writeBufferFile(const unsigned char* data){
    // name.sbuf
    // calculate name as ????
    // write to data/buffer/modelname/????.sbuf

    // element type (4)
    // component type (4)
    // byte length (8)
    // data (byte length)
}

void GLTFParser::writeMeshFile(){
    // index buffer id
    // material id
    // attribute ids
}


void GLTFParser::writeMaterialFile(){

}


void GLTFParser::writeModelFile(){

}


void GLTFParser::writeTextureFile(){

}

void GLTFParser::handleBuffer(
        const tinygltf::Buffer& buffer, 
        int byteOffset, 
        int byteLength){
    // write slice of buffer into new buffer
    const unsigned char* bufferData = buffer.data.data();
    const unsigned char* data = new unsigned char[byteLength];
    std::copy(bufferData+byteOffset, bufferData+byteOffset+byteLength, data);
    
    // write slice to file
    writeBufferFile(data);
}

void GLTFParser::handleBufferInterleaved(
        const tinygltf::Buffer& buffer, 
        int byteOffset, 
        int byteLength, 
        int byteStride,
        int bytesPerElement){
    std::vector<unsigned char> bufferData = buffer.data;
    std::vector<unsigned char> data;
    // iterate over buffer, one stride at a time 
    for (int i = byteOffset; i < byteOffset + byteLength; i+= byteStride){
        // grab neccessary bytes from stride (byte-by byte, may be slow)
        for (int b = 0; b < bytesPerElement; b++){
            data.push_back(bufferData[i+b]);
        }
    }

    // write data to file
    writeBufferFile(data.data());
}

void GLTFParser::handleBufferView(
        const tinygltf::BufferView& bufferView, 
        int byteOffset,
        int bytesPerElement, 
        int elementCount, 
        int elementType, 
        int componentType){
    // properties
    int adjustedByteOffset = bufferView.byteOffset + byteOffset;
    int byteLength = bufferView.byteLength;
    int byteStride = bufferView.byteStride;

    // buffer
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    // handle buffer
    if (byteStride == bytesPerElement)
        handleBuffer(buffer, byteOffset, byteLength);
    else
        handleBufferInterleaved(buffer, byteOffset, byteStride*elementCount, byteStride, bytesPerElement);
    
}

void GLTFParser::handleAccessor(const tinygltf::Accessor& accessor){
    // properties
    int byteOffset = accessor.byteOffset;
    int elementCount = accessor.count;
    int elementType = accessor.type;
    int componentType = accessor.componentType;
    int bytesPerElement = tinygltf::GetNumComponentsInType(elementType) * tinygltf::GetComponentSizeInBytes(componentType);

    // buffer view
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

    // handle bufer view
    handleBufferView(bufferView, byteOffset, bytesPerElement, elementCount, elementType, componentType);
}

void handleMaterial(const tinygltf::Material& material){
    // material flags
    int materialFlags = 0;

    // pbrmetallicroughness
    const tinygltf::PbrMetallicRoughness& pbr = material.pbrMetallicRoughness;
    if (pbr.baseColorTexture.index >= 0){ // base color
        materialFlags |= 0b1;
    }
    if (pbr.metallicRoughnessTexture.index >= 0){ //metallicroughness
        materialFlags |= 0b1<<1;
    }
    // normal
    const tinygltf::NormalTextureInfo& normal = material.normalTexture;
    if (normal.index >= 0){
        materialFlags |= 0b1<<2;
    }
    // occlusion
    const tinygltf::OcclusionTextureInfo& occlusion = material.occlusionTexture;
    if (occlusion.index >= 0){
        materialFlags |= 0b1<<3;
    }
    // emissive
    const tinygltf::TextureInfo& emissive = material.emissiveTexture;
    if (emissive.index >= 0){
        materialFlags |= 0b1<<4;
    }
    // alphamode
    std::string alpha = material.alphaMode;
    if (alpha != "OPAQUE"){
        materialFlags |= 0b1<<5;
    }
    // doublesided 
    if (material.doubleSided){
        materialFlags |= 0b1<<6;
    }
    //TODO: check for specularglossiness + update flags
}

void GLTFParser::handlePrimitive(const tinygltf::Primitive& primitive){
    // material
    int materialIndex = primitive.material;

    // attributes (accessor indices)
    int indicesAccessorIndex = primitive.indices;
    int positionAccessorIndex = -1;
    int normalAccessorIndex = -1;
    int tangentAccessorIndex = -1;
    std::vector<int> texcoordAccessorIndices;
    std::vector<int> colorAccessorIndices;

    // iterate over primitive's attributes
    for (auto const& [key, val] : primitive.attributes){
        if (key == "POSITION"){
            positionAccessorIndex = val;
        } else if (key == "NORMAL"){
            normalAccessorIndex = val;
        } else if (key == "TANGENT"){
            tangentAccessorIndex = val;
        } else if (key.find("TEXCOORD") != std::string::npos){
            texcoordAccessorIndices.push_back(val);
        } else if (key.find("COLOR") != std::string::npos){
            colorAccessorIndices.push_back(val);
        }
    }

    // handle accessors
    // indices
    handleAccessor(model.accessors[indicesAccessorIndex]);

    // position
    if (positionAccessorIndex >= 0){
        handleAccessor(model.accessors[positionAccessorIndex]);
    }

    // normal
    if (normalAccessorIndex >= 0){
        handleAccessor(model.accessors[normalAccessorIndex]);
    }

    // tangent
    if (tangentAccessorIndex >= 0){
        handleAccessor(model.accessors[tangentAccessorIndex]);
    }

    // texcoords
    for (auto & texcoordAccessorIndex : texcoordAccessorIndices) {
        handleAccessor(model.accessors[texcoordAccessorIndex]);
    }

    // colors
    for (auto & colorAccessorIndex : colorAccessorIndices) {
        handleAccessor(model.accessors[colorAccessorIndex]);
    }

    // handle material
    handleMaterial(model.materials[materialIndex]);

}

void GLTFParser::handleMesh(const tinygltf::Mesh& mesh){
    for (int i = 0; i < mesh.primitives.size(); i++){
        const tinygltf::Primitive& primitive = mesh.primitives[i];
        handlePrimitive(primitive);
    }
}

void GLTFParser::parseNode(const tinygltf::Node& node){
    // handle meshes
    if (node.mesh != -1){
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        handleMesh(mesh);
    }

    // handle children
    for (int i = 0; i < node.children.size(); i++){
        const tinygltf::Node& child = model.nodes[node.children[i]];
        parseNode(child);
    }
}

void GLTFParser::parse(){
    // assume one scene
    const tinygltf::Scene& scene = model.scenes[0];

    // process top level nodes
    for (int i = 0; i < scene.nodes.size(); i++){
        const tinygltf::Node& currNode = model.nodes[scene.nodes[i]];
        parseNode(currNode);
    }
}

// parse .gltf file
void GLTFParser::parseJson(std::string path){
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse .gltf\n");
        return;
    }

    parse();
}

// parse .glb file
void GLTFParser::parseBinary(std::string path){
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse .glb\n");
        return;
    }   

    m_path = std::filesystem::path(path).parent_path().concat("/");
    m_name = std::filesystem::path(path).stem();
    m_extension = std::filesystem::path(path).extension();

    parse();
}

}