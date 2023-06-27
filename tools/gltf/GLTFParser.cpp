#include <cstring>
#include <fstream>
#include "GLTFParser.h"

#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>
#include "ktx.h"
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#include "glm/gtx/string_cast.hpp"

#include "../../external/ktx/include/ktx.h"
#include "../../external/ktx/lib/vk_format.h"

namespace spr::tools{

GLTFParser::GLTFParser(){}

void GLTFParser::writeBufferFile(const unsigned char* data, std::string association, uint32_t byteLength, uint32_t elementType, uint32_t componentType, uint32_t bufferId){
    // association (4)
    // element type (4)
    // component type (4)
    // byte length (4)
    // data (byte length)

    // write to file
    // open file
    std::ofstream f("../data/buffers/"+(m_name+("_"+std::to_string(bufferId)))+".sbuf", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }

    // write association
    f.write(association.data(), sizeof(uint32_t));

    // write element type
    f.write((char*)&elementType, sizeof(uint32_t));

    // write component type
    f.write((char*)&componentType, sizeof(uint32_t));

    // write byte length
    f.write((char*)&byteLength, sizeof(uint32_t));
    
    // write data 
    f.write((char*)data, byteLength);

    // close file
    f.close();
}

void GLTFParser::writeMeshFile(
        uint32_t materialId, 
        uint32_t indexBufferId, 
        uint32_t posBufferId, 
        uint32_t attributesBufferId,
        uint32_t meshId){
    // material id (4)
    // index buffer id (4)
    // position buffer id (4)
    // attributes buffer id (4)


    // write to file
    // open file
    std::ofstream f("../data/meshes/"+(m_name+("_"+std::to_string(meshId)))+".smsh", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }

    // write material id
    f.write((char*)&materialId, sizeof(uint32_t));

    // write index buffer id
    f.write((char*)&indexBufferId, sizeof(uint32_t));

    // write position buffer id
    f.write((char*)&posBufferId, sizeof(uint32_t));

    // write attributes buffer id
    f.write((char*)&attributesBufferId, sizeof(uint32_t));

    // close file
    f.close();
}

void GLTFParser::writeMaterialFile(
        uint32_t materialFlags, 
        const tinygltf::Material& material,
        std::vector<uint32_t> texIds,
        uint32_t materialId){
    uint32_t currId = 0;
    // write to file
    // open file
    std::ofstream f("../data/materials/"+(m_name+("_"+std::to_string(materialId)))+".smtl", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }

    // base color (4)
    //      tex id (4) (2 id, 2 min/mag filter)
    //      factor (16)
    if (materialFlags & 0b1){
        // base color sentinel
        uint32_t sentinel = 1;
        f.write((char*)&sentinel, sizeof(uint32_t));
        
        // tex id
        uint32_t baseColorTexId = texIds[currId++];
        f.write((char*)&baseColorTexId, sizeof(uint32_t));
        
        // base color factor (x,y,z,w)
        std::vector<double> baseColorFactorDouble = material.pbrMetallicRoughness.baseColorFactor;
        std::vector<float> baseColorFactor(baseColorFactorDouble.begin(), baseColorFactorDouble.end());
        //      x
        f.write((char*)&baseColorFactor[0], sizeof(float));
        //      y
        f.write((char*)&baseColorFactor[1], sizeof(float));
        //      z
        f.write((char*)&baseColorFactor[2], sizeof(float));
        //      w
        f.write((char*)&baseColorFactor[3], sizeof(float));
    }

    // metalroughness (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      factor (8) (m(4)/r(4))
    if (materialFlags & (0b1<<1)){
        // metalroughness sentinel
        uint32_t sentinel = 2;
        f.write((char*)&sentinel, sizeof(uint32_t));
        
        // tex id
        uint32_t metalRoughnessTexId = texIds[currId++];
        f.write((char*)&metalRoughnessTexId, sizeof(uint32_t));
        
        // metalness factor
        float metalFactor = material.pbrMetallicRoughness.metallicFactor;
        f.write((char*)&metalFactor, sizeof(float));
        
        // roughness factor
        float roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
        f.write((char*)&roughnessFactor, sizeof(float));
    }

    // normal (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      scale (4)
    if (materialFlags & (0b1<<2)){
        // normal sentinel
        uint32_t sentinel = 3;
        f.write((char*)&sentinel, sizeof(uint32_t));
        
        // tex id
        uint32_t normalTexId = texIds[currId++];
        f.write((char*)&normalTexId, sizeof(uint32_t));
        
        // normal scale
        float normalScale = material.normalTexture.scale;
        f.write((char*)&normalScale, sizeof(float));
    }

    // occlusion (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      strength (4)
    if (materialFlags & (0b1<<3)){
        // occlusion sentinel
        uint32_t sentinel = 4;
        f.write((char*)&sentinel, sizeof(uint32_t));
        
        // tex id
        uint32_t occlusionTexId = texIds[currId++];
        f.write((char*)&occlusionTexId, sizeof(uint32_t));
        
        // occlusion strength
        float occlusionStrength = material.occlusionTexture.strength;
        f.write((char*)&occlusionStrength, sizeof(float));
    }

    // emissive (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      factor (12)
    if (materialFlags & (0b1<<4)){
        // emissive sentinel
        uint32_t sentinel = 5;
        f.write((char*)&sentinel, sizeof(uint32_t));

        // tex id
        uint32_t emissiveTexId = texIds[currId++];
        f.write((char*)&emissiveTexId, sizeof(uint32_t));

        // emissive factor (x,y,z);
        std::vector<double> emissiveFactorDouble = material.emissiveFactor;
        std::vector<float> emissiveFactor(emissiveFactorDouble.begin(), emissiveFactorDouble.end());
        //      x
        f.write((char*)&emissiveFactor[0], sizeof(float));
        //      y
        f.write((char*)&emissiveFactor[1], sizeof(float));
        //      z
        f.write((char*)&emissiveFactor[2], sizeof(float));
    }

    // alpha (4)
    //      type (4)
    //      cutoff (4)
    if (materialFlags & (0b1<<5)){
        // alpha sentinel
        uint32_t sentinel = 6;
        f.write((char*)&sentinel, sizeof(uint32_t));

        // alpha type
        uint32_t alphaType = 0; // blend by default
        if (material.alphaMode == "MASK")
            alphaType = 1;
        f.write((char*)&alphaType, sizeof(uint32_t));

        // alpha cutoff
        float alphaCutoff = material.alphaCutoff;
        f.write((char*)&alphaCutoff, sizeof(float));

    }

    // doublesided (4)
    if (materialFlags & (0b1<<6)){
        // doublesided sentinel
        uint32_t sentinel = 7;
        f.write((char*)&sentinel, sizeof(uint32_t));
    }

    // terminate
    uint32_t terminate = 0;
    f.write((char*)&terminate, sizeof(uint32_t));
    
    // close file
    f.close();
}

void GLTFParser::writeModelFile(uint32_t meshCount, std::vector<uint32_t> meshIds, uint32_t modelId){
    // assuming no animations

    // name (32)
    // mesh count (4)
    // mesh ids (4*count) 

    // write to file
    // open file
    std::ofstream f("../data/models/"+m_name+".smdl", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }

    // write name
    m_name.resize(32);
    f.write(m_name.c_str(), 32);

    // write mesh count
    f.write((char*)&meshCount, sizeof(uint32_t));

    // write mesh ids
    for (int i = 0; i < meshCount; i++){
        f.write((char*)&meshIds[i], sizeof(uint32_t));
    }

    // close file
    f.close();
}

void GLTFParser::writeTextureFile(uint32_t bufferId, uint32_t height, uint32_t width, uint32_t components, uint32_t texId){
    // buffer id (4)
    // height (4)
    // width (4)
    // components (4)


    // write to file
    // open file
    std::ofstream f("../data/textures/"+(m_name+("_"+std::to_string(texId)))+".stex", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }
    
    // write buffer id
    f.write((char*)&bufferId, sizeof(uint32_t));

    // write image height
    f.write((char*)&height, sizeof(uint32_t));

    // write image width
    f.write((char*)&width, sizeof(uint32_t));

    // write image components
    f.write((char*)&components, sizeof(uint32_t));

    // close file
    f.close();
}

void GLTFParser::handleBuffer(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t bytesPerElement,
        uint32_t elementCount,
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        glm::mat4& transform){    

    // check if we need to pad position to vec4, if it isn't already
    bool needsPadding = false;
    if (dataType == SPR_POSITION){
        assert (elementType == TINYGLTF_TYPE_VEC3 || elementType == TINYGLTF_TYPE_VEC4);
        if (elementType == TINYGLTF_TYPE_VEC3){
            byteLength = (4.f/3.f)*byteLength;
            elementType = TINYGLTF_TYPE_VEC4;
            needsPadding = true;
        }
    }

    // write slice of buffer into new buffer
    const unsigned char* bufferData = buffer.data.data();
    unsigned char* data = new unsigned char[byteLength];
    if (!needsPadding) {
        for (int32_t i = 0; i < byteLength; i++){
            data[i] = bufferData[i+byteOffset];
        }
    } else { // pad vec3 to vec4

        // get 1.0f as byte array
        char float1fByteArray[4];
        union {
            float f;
            unsigned char bytes[4];
        } floatToBytes;
        floatToBytes.f = 1.0f;
        memcpy(float1fByteArray, floatToBytes.bytes, 4);

        // copy data, padding with 1.0f
        uint32_t index = 0;
        for (int32_t i = 0; i < byteLength; i++){
            if (i%16 >= 12){ 
                // write 1.0f to pos.w
                data[i] = float1fByteArray[i%16 - 12];
            } else {
                // write buffer data to pos.xyz
                data[i] = bufferData[index+byteOffset];
                index++;
            }
        }
    }

    // apply transform to all positions
    if (dataType == SPR_POSITION){
        for (uint32_t i = 0; i < byteLength; i += 16){
            glm::vec4 pos = glm::make_vec4((float*)(data + i));
            pos = transform * pos;
            memcpy((unsigned char*)(data + i), ((unsigned char*)glm::value_ptr(pos)), 16);
        }
    }

    if (writeToFile){
        // write slice to file
        writeBufferFile(data, association.data(), byteLength, elementType, componentType, bufferId);
    } else {
        // write slice to 'out'
        out.resize(byteLength);
        memcpy(out.data(), data, byteLength);
    }

    delete[] data;
}



void GLTFParser::createMip(
        unsigned char* in, 
        uint32_t inSizeBytes, 
        uint32_t inExtent, 
        unsigned char* mipOut, 
        uint32_t outSizeBytes, 
        uint32_t outExtent)
{
    stbir_resize_uint8(in, inExtent, inExtent, 0, mipOut, outExtent, outExtent, 0, 4);
}

void GLTFParser::compressImageData(
        unsigned char* data,
        uint32_t dataSize,
        unsigned char** outData,
        uint32_t& outDataSize,
        BufferData dataType,
        uint32_t width,
        uint32_t height,
        uint32_t components)
{
    ktxTexture2* texture;
    ktxTextureCreateInfo createInfo;
    KTX_error_code result;
    ktx_uint32_t level, layer, faceSlice;
    ktx_size_t srcSize;
    ktx_size_t outSize;
    ktxBasisParams params = {0};
    params.structSize = sizeof(params);
    params.threadCount = 8;
    
    createInfo.glInternalformat = 0; 
    createInfo.vkFormat = dataType == SPR_TEXTURE_COLOR ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
    createInfo.baseWidth = width;
    createInfo.baseHeight = height;
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = std::floor(log2(std::max(width, height))) + 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 1;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;
    
    result = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
    if (result) {
        std::cerr << "Failed to create KTX2 texture, code: " << ktxErrorString(result) << std::endl;
    }

    uint32_t levels = createInfo.numLevels;
    uint32_t maxExtent = std::max(width, height);

    // base level
    srcSize = width*height*components;
    level = 0;
    layer = 0;
    faceSlice = 0;                           
    result = ktxTexture_SetImageFromMemory(ktxTexture(texture), level, layer, faceSlice, data, srcSize);
    if (result) {
        std::cerr << "Failed to set image from memory, code: " << ktxErrorString(result) << std::endl;
    }

    // mip chain
    unsigned char* mipData[levels-1];
    for (uint32_t i = 1; i < levels; i++){
        uint32_t levelExtent = std::max(maxExtent / (1 << i), 1u);
        mipData[i-1] = (unsigned char*)malloc(levelExtent*levelExtent*components);
    }

    uint32_t prevExtent = maxExtent;
    uint32_t prevSize = srcSize;
    for (uint32_t i = 1; i < levels; i++){
        uint32_t currExtent = std::max(maxExtent / (1 << i), 1u);
        uint32_t currSize = currExtent*currExtent*components;
        level = i;
        layer = 0;
        faceSlice = 0;                           

        createMip(i == 1 ? data : mipData[i-2], prevSize, prevExtent, mipData[i-1], currSize, currExtent);
        result = ktxTexture_SetImageFromMemory(ktxTexture(texture), level, layer, faceSlice, mipData[i-1], currSize);
        if (result) {
            std::cerr << "Failed to set (mip) image from memory, code: " << ktxErrorString(result) << std::endl;
        }

        prevExtent = currExtent;
        prevSize = currSize;

        if (currExtent == 1)
            break;
    }
    
    // BasisU encode
    if (dataType == SPR_TEXTURE_NORMAL){
        params.normalMap = KTX_TRUE;
        params.uastc = KTX_TRUE;
        params.uastcFlags = KTX_PACK_UASTC_LEVEL_DEFAULT;
        params.compressionLevel = KTX_ETC1S_DEFAULT_COMPRESSION_LEVEL;
    } else {
        params.uastc = KTX_FALSE;
        params.normalMap = KTX_FALSE;
        params.compressionLevel = KTX_ETC1S_DEFAULT_COMPRESSION_LEVEL;        
    }
    //result = ktxTexture2_CompressBasisEx(texture, &params);
    // if (result) {
    //     std::cerr << "Failed to compress texture, code: " << ktxErrorString(result) << std::endl;
    // }

    // cleanup
    result = ktxTexture_WriteToMemory((ktxTexture*)(texture), outData, &outSize);
    if (result) {
        std::cerr << "Failed to write KTX texture to memory, code: " << ktxErrorString(result) << std::endl;
    }
    outDataSize = (uint32_t)outSize;
    ktxTexture_Destroy(ktxTexture(texture));
    for (uint32_t i = 1; i < levels; i++){
        free(mipData[i-1]);
    }
}

void GLTFParser::handleTextureBuffer(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t bytesPerElement,
        uint32_t elementCount,
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        uint32_t width,
        uint32_t height,
        uint32_t components){    

    // write slice of buffer into new buffer
    const unsigned char* bufferData = buffer.data.data();
    unsigned char* data = new unsigned char[byteLength];
    for (int32_t i = 0; i < byteLength; i++){
        data[i] = bufferData[i+byteOffset];
    }

    unsigned char* ktxTextureData = nullptr;
    uint32_t ktxTextureDataSize;

    // generate mips + compress
    compressImageData(data, byteLength, &ktxTextureData, ktxTextureDataSize, dataType, width, height, 4);

    writeBufferFile(ktxTextureData, association.data(), ktxTextureDataSize, elementType, componentType, bufferId);

    free(ktxTextureData);
    delete[] data;
}

void GLTFParser::handleMIMEImageBuffer(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t bytesPerElement,
        uint32_t elementCount,
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId,
        BufferData dataType){

    // buffer data
    const unsigned char* bufferData = buffer.data.data();

    int width, height, numChannels;
    unsigned char* pixels = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(bufferData + byteOffset),
        byteLength,
        &width,
        &height,
        &numChannels,
        STBI_rgb_alpha
    );

    byteLength = width * height * STBI_rgb_alpha;
    
    unsigned char* data = new unsigned char[byteLength];
    for (int32_t i = 0; i < byteLength; i++)
        data[i] = pixels[i];
    free(pixels);

    unsigned char* ktxTextureData = nullptr;
    uint32_t ktxTextureDataSize;

    compressImageData(data, byteLength, &ktxTextureData, ktxTextureDataSize, dataType, width, height, 4); 

    writeBufferFile(ktxTextureData, association.data(), ktxTextureDataSize, elementType, componentType, bufferId);

    free(ktxTextureData);
    delete[] data;
}

void GLTFParser::handleBufferInterleaved(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t byteStride,
        uint32_t bytesPerElement, 
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId,
        std::vector<uint8_t>& out,
        bool writeToFile){
    std::vector<unsigned char> bufferData = buffer.data;
    std::vector<unsigned char> data;
    // iterate over buffer, one stride at a time 
    for (uint32_t i = byteOffset; i < byteOffset + byteLength; i+= byteStride){
        // grab neccessary bytes from stride (byte-by byte, may be slow)
        for (uint32_t b = 0; b < bytesPerElement; b++){
            data.push_back(bufferData[i+b]);
        }
    }

    if (writeToFile){
        // write data to file
        writeBufferFile(data.data(), association.data(), byteLength, elementType, componentType, bufferId);
    } else {
        // write data to 'out'
        out.resize(byteLength);
        memcpy(out.data(), data.data(), byteLength);
    }
    
}

void GLTFParser::handleBufferView(
        const tinygltf::BufferView& bufferView, 
        std::string association,
        uint32_t byteOffset,
        uint32_t bytesPerElement, 
        uint32_t elementCount, 
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        glm::mat4& transform){
    // properties
    uint32_t adjustedByteOffset = bufferView.byteOffset + byteOffset;
    uint32_t byteLength = bufferView.byteLength - byteOffset;
    uint32_t byteStride = bufferView.byteStride;
    if (byteStride == 0)
        byteStride = bytesPerElement;

    // buffer
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    // handle buffer
    if (byteStride == bytesPerElement){
        if (!association.compare("sbuf")) // normal case
            handleBuffer(buffer, association, adjustedByteOffset, byteLength, bytesPerElement, elementCount, elementType, componentType, bufferId, out, writeToFile, dataType, transform);
        else  // handle buffer that contains MIME image data
            handleMIMEImageBuffer(buffer, association, adjustedByteOffset, byteLength, bytesPerElement, elementCount, elementType, componentType, bufferId, dataType);
    } else
        handleBufferInterleaved(buffer, association, adjustedByteOffset, byteLength, byteStride, bytesPerElement, elementType, componentType, bufferId, out, writeToFile);
    
}

uint32_t GLTFParser::handleAccessor(const tinygltf::Accessor& accessor, std::vector<uint8_t>& out, bool writeToFile, BufferData dataType, glm::mat4& transform){
    uint32_t accessorId = m_id++;

    // properties
    uint32_t byteOffset = accessor.byteOffset;
    uint32_t elementCount = accessor.count;
    uint32_t elementType = accessor.type;
    uint32_t componentType = accessor.componentType;
    uint32_t bytesPerElement = tinygltf::GetNumComponentsInType(elementType) * tinygltf::GetComponentSizeInBytes(componentType);

    // buffer view
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

    // handle bufer view
    handleBufferView(bufferView, std::string("sbuf"), byteOffset, bytesPerElement, elementCount, elementType, componentType, accessorId, out, writeToFile, dataType, transform);
    return accessorId;
}

uint32_t GLTFParser::handleTexture(const tinygltf::Texture& tex, BufferData dataType){
    std::vector<uint8_t> out;
    
    // get image and sampler
    int32_t sourceIndex = tex.source;
    int32_t samplerIndex = tex.sampler;
    int32_t minFilter;
    tinygltf::Image image;
    tinygltf::Sampler sampler;
    if (sourceIndex == -1)
        return 0;
    if (samplerIndex == -1)
        minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
    else {
        sampler = model.samplers[samplerIndex];
        minFilter = sampler.minFilter;
    }

    // create masked id (id + filter)
    //uint32_t maskedTexId = (texId & 0xFFFF) | (minFilter << 16);

    // get image
    image = model.images[sourceIndex];

    // get image components
    uint32_t components = image.component;
    if (components == 0)
        components = 4;

    // tex already written to buffer
    // write tex file but not buffer
    if (m_sourceBuffIdMap.count(sourceIndex) > 0){
        // write texture to file
        return m_sourceTexIdMap[sourceIndex]; 
    }

    // get min filter
    minFilter = sampler.minFilter;

    // get data and write to buffer
    int32_t elementType = TINYGLTF_TYPE_VEC4;
    int32_t componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    int32_t texId = m_id++;
    int32_t bufferId = m_id++;
    m_sourceTexIdMap[sourceIndex] = texId;
    m_sourceBuffIdMap[sourceIndex] = bufferId;
    glm::mat4 temp;
    if (image.bufferView >= 0){ // bufferview
        int32_t elementCount = image.width * image.height * image.component;
        handleBufferView(model.bufferViews[image.bufferView], std::string("stex"), 0, 1, elementCount, elementType, componentType, bufferId, out, true, dataType, temp);
    } else { // direct buffer
        tinygltf::Buffer buffer;
        int32_t elementCount = image.image.size();
        int32_t elementType = TINYGLTF_TYPE_SCALAR;
        int32_t componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        buffer.data = image.image;
        uint32_t bytesPerElement = tinygltf::GetNumComponentsInType(elementType) * tinygltf::GetComponentSizeInBytes(componentType);
        handleTextureBuffer(buffer, std::string("stex"), 0, bytesPerElement*elementCount, bytesPerElement, elementCount, elementType, componentType, bufferId, out, true, dataType, image.width, image.height, components);
    }

    // write texture to file
    writeTextureFile(bufferId, image.height, image.width, components, texId);
    return texId;
}

uint32_t GLTFParser::handleTexture(const tinygltf::TextureInfo& texInfo, BufferData dataType){
    // get texture
    int32_t texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex, dataType);
}

uint32_t GLTFParser::handleTexture(const tinygltf::NormalTextureInfo& texInfo){
    // get texture
    int32_t texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex, SPR_TEXTURE_NORMAL);
}

uint32_t GLTFParser::handleTexture(const tinygltf::OcclusionTextureInfo& texInfo){
    // get texture
    int32_t texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex, SPR_TEXTURE_OTHER);
}

uint32_t GLTFParser::handleMaterial(const tinygltf::Material& material){
    uint32_t materialId = m_id++;
    // material flags
    uint32_t materialFlags = 0;

    // tex ids
    std::vector<uint32_t> texIds;

    // pbrmetallicroughness
    const tinygltf::PbrMetallicRoughness& pbr = material.pbrMetallicRoughness;
    if (pbr.baseColorTexture.index >= 0){ // base color
        materialFlags |= 0b1;
        texIds.push_back(handleTexture(pbr.baseColorTexture, SPR_TEXTURE_COLOR));
    }
    if (pbr.metallicRoughnessTexture.index >= 0){ //metallicroughness
        materialFlags |= (0b1<<1);
        texIds.push_back(handleTexture(pbr.metallicRoughnessTexture, SPR_TEXTURE_OTHER));
    }
    // normal
    const tinygltf::NormalTextureInfo& normal = material.normalTexture;
    if (normal.index >= 0){
        materialFlags |= (0b1<<2);
        texIds.push_back(handleTexture(normal));
    }
    // occlusion
    const tinygltf::OcclusionTextureInfo& occlusion = material.occlusionTexture;
    if (occlusion.index >= 0){
        materialFlags |= (0b1<<3);
        texIds.push_back(handleTexture(occlusion));
    }
    // emissive
    const tinygltf::TextureInfo& emissive = material.emissiveTexture;
    if (emissive.index >= 0){
        materialFlags |= (0b1<<4);
        texIds.push_back(handleTexture(emissive, SPR_TEXTURE_OTHER));
    }
    // alphamode
    std::string alpha = material.alphaMode;
    if (alpha != "OPAQUE"){
        materialFlags |= (0b1<<5);
    }
    // doublesided 
    if (material.doubleSided){
        materialFlags |= (0b1<<6);
    }
    //TODO: check for specularglossiness + update flags
    // write material to file
    writeMaterialFile(materialFlags, material, texIds, materialId);

    return materialId;
}

uint32_t GLTFParser::interleaveVertexAttributes(
        uint32_t vertexCount,
        std::vector<uint8_t>& normalBuffer,
        std::vector<uint8_t>& tangentBuffer,
        std::vector<uint8_t>& texCoordBuffer,
        std::vector<uint8_t>& colorBuffer,
        glm::mat4& transform){
    uint32_t attributesId = m_id++;

    uint32_t bytesPerNormal = 12;
    uint32_t bytesPerColor = 12;
    uint32_t bytesPerTexCoord = 8;
    uint32_t bytesPerVertex = bytesPerNormal + bytesPerColor + bytesPerTexCoord;

    if (normalBuffer.size() != vertexCount * bytesPerNormal){
        normalBuffer.resize(vertexCount * bytesPerNormal);
    }
    if (colorBuffer.size() != vertexCount * bytesPerColor){
        colorBuffer.resize(vertexCount * bytesPerColor);
        glm::vec3 defaultColor = {1.f, 1.f, 1.f};
        for (uint32_t i = 0; i < colorBuffer.size(); i+= bytesPerColor){
            memcpy(((unsigned char*)colorBuffer.data() + i), (unsigned char*)glm::value_ptr(defaultColor), bytesPerColor);
        }
    }
    if (texCoordBuffer.size() != vertexCount * bytesPerTexCoord){
        texCoordBuffer.resize(vertexCount * bytesPerTexCoord);
    }

    // transform normals
    for (uint32_t i = 0; i < vertexCount*bytesPerNormal; i += bytesPerNormal){
        glm::vec3 normal = glm::make_vec3((float*)(normalBuffer.data() + i));
        normal = glm::mat3(transform) * normal;
        memcpy((unsigned char*)(normalBuffer.data() + i), ((unsigned char*)glm::value_ptr(normal)), bytesPerNormal);
    }

    // interleave into attributes buffer
    std::vector<uint8_t> result(vertexCount*bytesPerVertex);
    // for each vertex, manually copy into 'result' such that it takes the form:
    //
    //      [ vec3 | vec2.x ]    OR    [ normal | texCoord.U ]
    //      [ vec3 | vec2.y ]          [ color  | texCoord.V ]
    //
    
    for (uint32_t vertex = 0; vertex < vertexCount; vertex++){
        uint32_t offset = vertex*bytesPerVertex;
        // copy normal 
        for(uint32_t normal = 0; normal < bytesPerNormal; normal++){
            result[offset + normal] = normalBuffer[vertex*bytesPerNormal + normal];
        }
        offset += bytesPerNormal;

        // copy texCoord.U
        for(uint32_t tex = 0; tex < bytesPerTexCoord/2; tex++){
            result[offset + tex] = texCoordBuffer[vertex*bytesPerTexCoord + tex];
        }
        offset += bytesPerTexCoord/2;

        // copy color
        for(uint32_t color = 0; color < bytesPerColor; color++){
            result[offset + color] = colorBuffer[vertex*bytesPerColor + color];
        }
        offset += bytesPerColor;
        
        // copy texCoord.V
        for(uint32_t tex = 0; tex < bytesPerTexCoord/2; tex++){
            result[offset + tex] = texCoordBuffer[vertex*bytesPerTexCoord + tex + bytesPerTexCoord/2];
        }
        offset += bytesPerTexCoord/2;
        
    }

    // write to buffer
    writeBufferFile(result.data(), std::string("sbuf"), vertexCount*bytesPerVertex, TINYGLTF_TYPE_VEC4, TINYGLTF_COMPONENT_TYPE_FLOAT, attributesId);

    return attributesId;
}

uint32_t GLTFParser::handlePrimitive(const tinygltf::Primitive& primitive, glm::mat4& transform){
    uint32_t meshId = m_id++;
    std::vector<uint8_t> tempOut;

    // material
    int32_t materialIndex = primitive.material;

    // attributes (accessor indices)
    int32_t indicesAccessorIndex = primitive.indices;
    int32_t positionAccessorIndex = -1;
    int32_t normalAccessorIndex = -1;
    int32_t tangentAccessorIndex = -1;
    int32_t texcoordAccessorIndex = -1;
    int32_t colorAccessorIndex = -1;

    // iterate over primitive's attributes
    for (auto const& [key, val] : primitive.attributes){
        if (key == "POSITION"){
            positionAccessorIndex = val;
        } else if (key == "NORMAL"){
            normalAccessorIndex = val;
        } else if (key == "TANGENT"){
            tangentAccessorIndex = val;
        } else if (key == "TEXCOORD_0"){
            texcoordAccessorIndex = val;
        } else if (key == "COLOR_0"){
            colorAccessorIndex = val;
        }
    }

    // handle accessors
    // indices
    int32_t indicesId = handleAccessor(model.accessors[indicesAccessorIndex], tempOut, true, SPR_INDICES, transform);
    assert(indicesId >= 0);
    
    // position
    int32_t positionId = -1;
    uint32_t vertexCount = 0;
    if (positionAccessorIndex >= 0){
        vertexCount = model.accessors[positionAccessorIndex].count;
        positionId = handleAccessor(model.accessors[positionAccessorIndex], tempOut, true, SPR_POSITION, transform);
    }
    assert(positionId >= 0);

    // normal
    int32_t normalId = -1;
    std::vector<uint8_t> outNormal;
    if (normalAccessorIndex >= 0){
        normalId = handleAccessor(model.accessors[normalAccessorIndex], outNormal, false, SPR_NORMALS, transform);
    }

    // tangent
    int32_t tangentId = -1;
    std::vector<uint8_t> outTangent;
    if (tangentAccessorIndex >= 0){
        tangentId = handleAccessor(model.accessors[tangentAccessorIndex], outTangent, false, SPR_TANGENTS, transform);
    }

    // texcoords
    int32_t texCoordId = -1;
    std::vector<uint8_t> outTexCoord;
    if (texcoordAccessorIndex >= 0) {
        texCoordId = handleAccessor(model.accessors[texcoordAccessorIndex], outTexCoord, false, SPR_UV, transform);
    }

    // colors
    int32_t colorId = -1;
    std::vector<uint8_t> outColor;
    if (colorAccessorIndex >= 0) {
        colorId = handleAccessor(model.accessors[colorAccessorIndex], outColor, false, SPR_COLOR, transform);
    }

    uint32_t attributesId = interleaveVertexAttributes(vertexCount, outNormal, outTangent, outTexCoord, outColor, transform);

    // handle material
    uint32_t materialId = 0;
    if (materialIndex >= 0)
        materialId = handleMaterial(model.materials[materialIndex]);

    // write prim (mesh) to file
    writeMeshFile(materialId, indicesId, positionId, attributesId, meshId);
    return meshId;
}

void GLTFParser::handleMesh(const tinygltf::Mesh& mesh, std::vector<uint32_t> &meshIds, glm::mat4& transform){
    for (int32_t i = 0; i < mesh.primitives.size(); i++){
        const tinygltf::Primitive& primitive = mesh.primitives[i];
        if (primitive.mode == 4 || primitive.mode == -1){
            meshIds.push_back(handlePrimitive(primitive, transform));
        }
    }
}

void GLTFParser::vectorToMat4(const std::vector<double>& src, glm::mat4& dst){
    float mat[16];
    for (uint32_t i = 0; i < 16; i++){
        mat[i] = (float)src.at(i);
    }
    dst = glm::make_mat4(mat);
}   

void GLTFParser::vectorToQuat(const std::vector<double>& src, glm::quat& dst){
    float quat[4];
    for (uint32_t i = 0; i < 4; i++){
        quat[i] = (float)src.at(i);
    }
    dst = glm::make_quat(quat);
}

void GLTFParser::vectorToVec3(const std::vector<double>& src, glm::vec3& dst){
    float vec[3];
    for (uint32_t i = 0; i < 3; i++){
        vec[i] = (float)src.at(i);
    }
    dst = glm::make_vec3(vec);
}

void GLTFParser::parseNode(const tinygltf::Node& node, std::vector<uint32_t> &meshIds, glm::mat4& transform){
    // get node transform matrix
    glm::mat4 nodeTransform = glm::mat4(1.0f);
    if (node.matrix.size() != 0){
        vectorToMat4(node.matrix, nodeTransform);
        //glm::transpose(nodeTransform);
    } else if (node.translation.size() || node.rotation.size() || node.translation.size()) {
        glm::vec3 t = {0.f, 0.f, 0.f};
        glm::quat r = {1.f, 0.f, 0.f, 0.f};
        glm::vec3 s = {1.f, 1.f, 1.f};
        
        if (node.translation.size())
            vectorToVec3(node.translation, t);
        if (node.rotation.size())
            vectorToQuat(node.rotation, r);
        if (node.scale.size())
            vectorToVec3(node.scale, s);

        glm::mat4 translation = glm::translate(glm::mat4(1.f), t);
        glm::mat4 rotation = glm::toMat4(r);
        glm::mat4 scale = glm::scale(glm::mat4(1.f), s);

        nodeTransform = translation * rotation * scale;
    }

    // apply parent transform
    nodeTransform = transform * nodeTransform;

    // handle meshes
    if (node.mesh != -1){
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        handleMesh(mesh, meshIds, nodeTransform);
    }

    // handle children
    for (int32_t i = 0; i < node.children.size(); i++){
        const tinygltf::Node& child = model.nodes[node.children[i]];
        parseNode(child, meshIds, nodeTransform);
    }
}

void GLTFParser::parse(){

    int32_t modelId = m_id++;
    // assume one scene
    const tinygltf::Scene& scene = model.scenes[0];
    std::vector<uint32_t> meshIds;
    meshIds.reserve(256);
    // process top level nodes
    for (int32_t i = 0; i < scene.nodes.size(); i++){
        const tinygltf::Node& currNode = model.nodes[scene.nodes[i]];
        glm::mat4 identity = glm::mat4(1.0f);
        parseNode(currNode, meshIds, identity);
    }
    // write model file
    writeModelFile(meshIds.size(), meshIds, modelId);
}

// parse .gltf file
void GLTFParser::parseJson(std::string path){
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
    }

    if (!err.empty()) {
    }

    if (!ret) {
        return;
    }

    m_path = std::filesystem::path(path).parent_path().concat("/");
    m_name = std::filesystem::path(path).stem();
    m_extension = std::filesystem::path(path).extension();


    parse();
}

// parse .glb file
void GLTFParser::parseBinary(std::string path){
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
    }

    if (!err.empty()) {
    }

    if (!ret) {
        return;
    }   

    m_path = std::filesystem::path(path).parent_path().concat("/");
    m_name = std::filesystem::path(path).stem();
    m_extension = std::filesystem::path(path).extension();


    parse();
}

}