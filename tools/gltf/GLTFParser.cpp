#include <cstring>
#include <fstream>
#include "GLTFParser.h"
#include <initializer_list>
#include <stdio.h>
#include "Resources.h"
#include "SprLog.h"
#include "util/Span.h"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <vulkan/vulkan_core.h>
#include "ktx.h"
//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE STBIR_FILTER_MITCHELL
#include "stb_image_resize.h"
#include "glm/gtx/string_cast.hpp"

#include "../../external/ktx/include/ktx.h"
#include "../../external/ktx/lib/vk_format.h"
#include "external/flat_hash_map/flat_hash_map.hpp"

namespace spr::tools{

GLTFParser::GLTFParser(){}

OffsetSpan GLTFParser::writeBufferFile(const unsigned char* data, uint32 byteLength, DataRegion dataRegion){
    OffsetSpan offsetSpan;
    if (dataRegion == SPR_DR_INDEX){
        m_indexDataStream.write((char*)data, byteLength);
        offsetSpan = {byteLength, m_indicesOffset};
        m_indicesOffset += byteLength;
    } else if (dataRegion == SPR_DR_POSITION){
        m_positionDataStream.write((char*)data, byteLength);
        offsetSpan = {byteLength, m_positionOffset};
        m_positionOffset += byteLength;
    } else if (dataRegion == SPR_DR_ATTRIBUTE){
        m_attributeDataStream.write((char*)data, byteLength);
        offsetSpan = {byteLength, m_attributesOffset};
        m_attributesOffset += byteLength;
    } else if (dataRegion == SPR_DR_TEXTURE){
        m_textureDataStream.write((char*)data, byteLength);
        offsetSpan = {byteLength, m_textureDataOffset};
        m_textureDataOffset += byteLength;
    } else {
        offsetSpan = {0, 0};
    }

    return offsetSpan;
}

uint32 GLTFParser::writeTextureFile(TextureLayout& texture){
    m_textureStream.write((char*)&texture, sizeof(TextureLayout));
    m_textureCount++;
    return m_textureIndex++;
}

uint32 GLTFParser::writeMaterialFile(MaterialLayout& material){
    m_materialStream.write((char*)&material, sizeof(MaterialLayout));
    m_materialCount++;
    return m_materialIndex++;
}

uint32 GLTFParser::writeMeshFile(MeshLayout& mesh){
    m_meshStream.write((char*)&mesh, sizeof(MeshLayout));
    m_meshCount++;
    return m_meshIndex++;
}


void GLTFParser::createMip(
        unsigned char* in, 
        uint32 inSizeBytes, 
        uint32 inExtent, 
        unsigned char* mipOut, 
        uint32 outSizeBytes, 
        uint32 outExtent,
        uint32 components)
{
    stbir_resize_uint8(in, inExtent, inExtent, 0, mipOut, outExtent, outExtent, 0, components);
}

VkFormat getFormat(BufferData dataType){
    if(dataType == SPR_TEXTURE_COLOR){
        return VK_FORMAT_R8G8B8A8_SRGB;
    } else if (dataType == SPR_TEXTURE_NORMAL){
        return VK_FORMAT_R8G8B8A8_UNORM;
    } else if (dataType == SPR_TEXTURE_MR){
        return VK_FORMAT_R8G8B8A8_UNORM;
    } else if (dataType == SPR_TEXTURE_EMISSIVE){
        return VK_FORMAT_R8G8B8A8_SRGB;
    } else if (dataType == SPR_TEXTURE_OCCLUSION){
        return VK_FORMAT_R8G8B8A8_UNORM;
    } else {
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
}


uint32 getComponents(BufferData dataType){
    if(dataType == SPR_TEXTURE_COLOR){
        return 4;
    } else if (dataType == SPR_TEXTURE_NORMAL){
        return 4;
    } else if (dataType == SPR_TEXTURE_MR){
        return 4;
    } else if (dataType == SPR_TEXTURE_EMISSIVE){
        return 4;
    } else if (dataType == SPR_TEXTURE_OCCLUSION){
        return 4;
    } else {
        return 4;
    }
}

void GLTFParser::compressImageData(
        unsigned char* data,
        uint32 dataSize,
        unsigned char** outData,
        uint32& outDataSize,
        BufferData dataType,
        uint32 width,
        uint32 height,
        uint32 components)
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
    createInfo.vkFormat = getFormat(dataType),
    createInfo.baseWidth = width;
    createInfo.baseHeight = height;
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 2;
    createInfo.numLevels = width == height ? std::floor(std::log2(std::max(width, height))) + 1 : 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 1;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;
    
    result = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
    if (result) {
        SprLog::error({{"Failed to create KTX2 texture, code: "}, {ktxErrorString(result)}});
    }

    uint32 levels = createInfo.numLevels;
    uint32 maxExtent = std::max(width, height);

    // base level
    components = getComponents(dataType);
    srcSize = width*height*components;
    level = 0;
    layer = 0;
    faceSlice = 0;                           
    result = ktxTexture_SetImageFromMemory(ktxTexture(texture), level, layer, faceSlice, data, srcSize);
    if (result) {
        SprLog::error({{"Failed to set image from memory, code: "}, {ktxErrorString(result)}});
    }

    // mip chain
    unsigned char* mipData[levels-1];
    if (width == height){
        for (uint32 i = 1; i < levels; i++){
            uint32 levelExtent = std::max(maxExtent / (1 << i), 1u);
            mipData[i-1] = (unsigned char*)malloc(levelExtent*levelExtent*components);
        }

        uint32 prevExtent = maxExtent;
        uint32 prevSize = srcSize;
        for (uint32 i = 1; i < levels; i++){
            uint32 currExtent = std::max(maxExtent / (1 << i), 1u);
            uint32 currSize = currExtent*currExtent*components;
            level = i;
            layer = 0;
            faceSlice = 0;                           

            createMip(i == 1 ? data : mipData[i-2], prevSize, prevExtent, mipData[i-1], currSize, currExtent, components);
            result = ktxTexture_SetImageFromMemory(ktxTexture(texture), level, layer, faceSlice, mipData[i-1], currSize);
            if (result) {
                SprLog::error({{"Failed to set (mip) image from memory, code: "}, {ktxErrorString(result)}});
            }

            prevExtent = currExtent;
            prevSize = currSize;

            if (currExtent <= 1)
                break;
        }
    }

    // cleanup
    result = ktxTexture_WriteToMemory((ktxTexture*)(texture), outData, &outSize);
    if (result) {
        SprLog::error({{"Failed to write KTX texture to memory, code: "}, {ktxErrorString(result)}});
    }
    outDataSize = (uint32)outSize;
    ktxTexture_Destroy(ktxTexture(texture));
    if (width == height){
        for (uint32 i = 1; i < levels; i++){
            free(mipData[i-1]);
        }
    }
}


OffsetSpan GLTFParser::handleTextureBuffer(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32 byteOffset, 
        uint32 byteLength, 
        uint32 bytesPerElement,
        uint32 elementCount,
        uint32 elementType, 
        uint32 componentType,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        uint32 width,
        uint32 height,
        uint32 components){    
    // write slice of buffer into new buffer
    const unsigned char* bufferData = buffer.data.data();

    unsigned char* ktxTextureData = nullptr;
    uint32 ktxTextureDataSize;

    // generate mips + compress
    int targetComponents = getComponents(dataType);
    compressImageData((unsigned char*)(bufferData + byteOffset), byteLength, &ktxTextureData, ktxTextureDataSize, dataType, width, height, targetComponents);

    OffsetSpan offsetSpan = writeBufferFile(ktxTextureData, ktxTextureDataSize, SPR_DR_TEXTURE);

    free(ktxTextureData);
    
    return offsetSpan;
}

OffsetSpan GLTFParser::handleMIMEImageBuffer(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32 byteOffset, 
        uint32 byteLength, 
        uint32 bytesPerElement,
        uint32 elementCount,
        uint32 elementType, 
        uint32 componentType,
        BufferData dataType){

    // buffer data
    const unsigned char* bufferData = buffer.data.data();
    int width, height, numChannels;
    int components = getComponents(dataType);
    unsigned char* pixels = stbi_load_from_memory(
        reinterpret_cast<const stbi_uc*>(bufferData + byteOffset),
        byteLength,
        &width,
        &height,
        &numChannels,
        components
    );

    byteLength = width * height * components;    

    unsigned char* ktxTextureData = nullptr;
    uint32 ktxTextureDataSize;

    compressImageData(pixels, byteLength, &ktxTextureData, ktxTextureDataSize, dataType, width, height, components); 

    OffsetSpan offsetSpan = writeBufferFile(ktxTextureData, ktxTextureDataSize, SPR_DR_TEXTURE);

    free(ktxTextureData);
    free(pixels);

    return offsetSpan;
}


void padAndTransformData(const unsigned char* bufferData, 
        unsigned char* data,
        uint32 byteOffset, 
        uint32 byteLength, 
        uint32 bytesPerElement,
        uint32 elementCount,
        uint32 elementType, 
        uint32 componentType,
        std::vector<uint8_t>& out,
        BufferData dataType,
        glm::mat4& transform){
            
    // check if we need to pad position to vec4, if it isn't already
    bool needsPosPadding = false;
    if (dataType == SPR_POSITION){
        assert (elementType == TINYGLTF_TYPE_VEC3 || elementType == TINYGLTF_TYPE_VEC4);
        if (elementType == TINYGLTF_TYPE_VEC3){
            needsPosPadding = true;
        }
    }

    bool needsIndicesPadding = false;
    if (dataType == SPR_INDICES){
        if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT){
            needsIndicesPadding = true;
        }
    }

    if (needsPosPadding) { // pad vec3 to vec4
        uint32 offset = 0;
        for (uint32 i = 0; i < bytesPerElement*elementCount; i += sizeof(glm::vec3)){
            glm::vec3 pos = glm::make_vec3((float*)(bufferData + byteOffset + i));
            glm::vec4 p = {pos.x, pos.y, pos.z, 1.f};
            memcpy((unsigned char*)(data + offset), ((unsigned char*)glm::value_ptr(p)), BYTES_PER_POSITION);
            offset += BYTES_PER_POSITION;
        }
    } else if (needsIndicesPadding){ // pad uint16 to uint32
        uint32 offset = 0;
        for (int32 i = 0; i < bytesPerElement*elementCount; i += sizeof(uint16)){
            uint16 index = ((uint16*)(bufferData + byteOffset + i))[0];
            uint32 idx = index;
            memcpy((unsigned char*)(data + offset), (unsigned char*)(&idx), sizeof(uint32));
            offset += sizeof(uint32);
        }
    } else { // no padding, just copy
        memcpy(data, bufferData + byteOffset, byteLength);
    }
    
    // apply transform to all positions
    if (dataType == SPR_POSITION){
        for (uint32 i = 0; i < byteLength; i += BYTES_PER_POSITION){
            glm::vec4 pos = glm::make_vec4((float*)(data + i));
            pos = transform * pos;
            memcpy((unsigned char*)(data + i), ((unsigned char*)glm::value_ptr(pos)), BYTES_PER_POSITION);
        }
    }
}


OffsetSpan GLTFParser::handleBuffer(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32 byteOffset, 
        uint32 byteLength, 
        uint32 bytesPerElement,
        uint32 elementCount,
        uint32 elementType, 
        uint32 componentType,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        glm::mat4& transform,
        DataRegion region){    
    
    // check if we need to pad position to vec4, if it isn't already
    if (dataType == SPR_POSITION){
        assert (elementType == TINYGLTF_TYPE_VEC3 || elementType == TINYGLTF_TYPE_VEC4);
        if (elementType == TINYGLTF_TYPE_VEC3){
            byteLength = (4.f/3.f)*byteLength;
        }
    }

    if (dataType == SPR_INDICES){
        if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT){
            byteLength = 2*byteLength;
        }
    }

    // write slice of buffer into new buffer
    const unsigned char* bufferData = buffer.data.data();
    unsigned char* data = new unsigned char[byteLength];
    
    padAndTransformData(bufferData, data, byteOffset, byteLength, bytesPerElement, elementCount, elementType, componentType, out, dataType, transform);

    // write slice to 'out'
    out.resize(byteLength);
    memcpy(out.data(), data, byteLength);

    OffsetSpan offset;
    if (writeToFile){
        offset = writeBufferFile(data, byteLength, region);
    }

    delete[] data;
    return offset;
}



OffsetSpan GLTFParser::handleBufferInterleaved(
        const tinygltf::Buffer& buffer, 
        std::string association,
        uint32 byteOffset, 
        uint32 byteLength, 
        uint32 byteStride,
        uint32 bytesPerElement, 
        uint32 elementType, 
        uint32 componentType,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        glm::mat4& transform,
        DataRegion region){
    std::vector<unsigned char> bufferData = buffer.data;
    std::vector<unsigned char> sequentialBufferData;
    std::vector<unsigned char> data;

    // iterate over buffer, one stride at a time
    uint32 elementCount = byteLength / bytesPerElement;
    for (uint32 i = byteOffset; i < byteOffset + elementCount * byteStride; i+= byteStride){
        // grab neccessary bytes from stride (byte-by byte, may be slow)
        for (uint32 b = 0; b < bytesPerElement; b++){
            sequentialBufferData.push_back(bufferData[i+b]);
        }
    }

    // check if we need to pad position to vec4, if it isn't already
    if (dataType == SPR_POSITION){
        assert (elementType == TINYGLTF_TYPE_VEC3 || elementType == TINYGLTF_TYPE_VEC4);
        if (elementType == TINYGLTF_TYPE_VEC3){
            byteLength = (4.f/3.f)*byteLength;
        }
    }

    if (dataType == SPR_INDICES){
        if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT){
            byteLength = 2*byteLength;
        }
    }
    data.resize(bufferData.size());
    
    padAndTransformData(sequentialBufferData.data(), data.data(), byteOffset, byteLength, bytesPerElement, elementCount, elementType, componentType, out, dataType, transform);

    // write data to 'out'
    out.reserve(byteLength);
    memcpy(out.data(), data.data(), byteLength);

    if (writeToFile){
        return writeBufferFile(data.data(), byteLength, region);
    } else {
        return {0,0};
    }
}

OffsetSpan GLTFParser::handleBufferView(
        const tinygltf::BufferView& bufferView, 
        std::string association,
        uint32 byteOffset,
        uint32 bytesPerElement, 
        uint32 elementCount, 
        uint32 elementType, 
        uint32 componentType,
        std::vector<uint8_t>& out,
        bool writeToFile,
        BufferData dataType,
        glm::mat4& transform,
        DataRegion region){
    // properties

    uint32 adjustedByteOffset = bufferView.byteOffset + byteOffset;
    uint32 byteLength = elementCount * bytesPerElement;
    uint32 byteStride = bufferView.byteStride;

    if (byteStride == 0)
        byteStride = bytesPerElement;

    // buffer
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    // handle buffer
    OffsetSpan offset;
    if (byteStride == bytesPerElement){
        if (!association.compare("sbuf")){ // normal case
            offset = handleBuffer(buffer, association, adjustedByteOffset, byteLength, bytesPerElement, elementCount, elementType, componentType, out, writeToFile, dataType, transform, region);
        }else{  // handle buffer that contains MIME image data
            return handleMIMEImageBuffer(buffer, association, adjustedByteOffset, byteLength, bytesPerElement, elementCount, elementType, componentType, dataType);
        }
    } else{
        offset = handleBufferInterleaved(buffer, association, adjustedByteOffset, byteLength, byteStride, bytesPerElement, elementType, componentType, out, writeToFile, dataType, transform, region);
    }    
    return offset;
}



uint32 GLTFParser::handleTexture(const tinygltf::Texture& tex, BufferData dataType){
    std::vector<uint8_t> out;
    // get image and sampler
    int32 sourceIndex = tex.source;
    int32 samplerIndex = tex.sampler;
    
    tinygltf::Sampler sampler;
    if (sourceIndex == -1){
        return 0;
    }
    if (samplerIndex != -1) {
        sampler = model.samplers[samplerIndex];
    }

    // get image
    tinygltf::Image& image = model.images[sourceIndex];

    // get image components
    uint32 components = image.component;
    if (components == 0)
        components = 4;

    // tex already written to buffer,
    // write tex file but not buffer
    if (m_sourceBuffIdMap.count(sourceIndex) > 0){
        // write texture to file
        return m_sourceTexIdMap[sourceIndex]; 
    }

    // get min filter
    //minFilter = sampler.minFilter;

    // get data and write to buffer
    int32 elementType = TINYGLTF_TYPE_VEC4;
    int32 componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    glm::mat4 temp;

    OffsetSpan textureOffset;
    if (image.bufferView >= 0){ // bufferview
        if (m_bufferviewMap.count(image.bufferView) > 0){
            return m_bufferviewMap[image.bufferView];
        }
        int32 elementCount = image.width * image.height * image.component;
        textureOffset = handleBufferView(model.bufferViews[image.bufferView], std::string("stex"), 0, 1, elementCount, elementType, componentType, out, true, dataType, temp, SPR_DR_TEXTURE);
        m_bufferviewMap[image.bufferView] = m_textureIndex;
    } else { // direct buffer
        uint64_t hash = hashVec((uint32*)image.image.data(), image.image.size()/sizeof(uint32)-(image.image.size()%sizeof(uint32)));
        if (m_imageMap.count(hash) > 0){
            return m_imageMap[hash];
        }
        tinygltf::Buffer buffer;
        int32 elementCount = image.image.size();
        int32 elementType = TINYGLTF_TYPE_SCALAR;
        int32 componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        buffer.data = image.image;
        uint32 bytesPerElement = tinygltf::GetNumComponentsInType(elementType) * tinygltf::GetComponentSizeInBytes(componentType);
        textureOffset = handleTextureBuffer(buffer, std::string("stex"), 0, bytesPerElement*elementCount, bytesPerElement, elementCount, elementType, componentType, out, true, dataType, image.width, image.height, components);
        m_imageMap[hash] = m_textureIndex;
    }
    m_sourceTexIdMap[sourceIndex] = m_textureIndex;
    m_sourceBuffIdMap[sourceIndex] = sourceIndex;
    

    // write texture to file
    TextureLayout texture {
        .id = 0,
        .dataSizeBytes = textureOffset.sizeBytes, 
        .dataOffset = textureOffset.offset, 
        .dataBufferId = 0,
        .height = (uint32)image.height, 
        .width = (uint32)image.width, 
        .components = components
    };
    return writeTextureFile(texture);
}

uint32 GLTFParser::handleTexture(const tinygltf::TextureInfo& texInfo, BufferData dataType){
    // get texture
    int32 texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex, dataType);
}

uint32 GLTFParser::handleTexture(const tinygltf::NormalTextureInfo& texInfo){
    // get texture
    int32 texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex, SPR_TEXTURE_NORMAL);
}

uint32 GLTFParser::handleTexture(const tinygltf::OcclusionTextureInfo& texInfo){
    // get texture
    int32 texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex, SPR_TEXTURE_OCCLUSION);
}

uint32 GLTFParser::handleMaterial(const tinygltf::Material& material, uint32& outMaterialFlags){
    const tinygltf::PbrMetallicRoughness& pbr = material.pbrMetallicRoughness;
    const tinygltf::NormalTextureInfo& normal = material.normalTexture;
    const tinygltf::OcclusionTextureInfo& occlusion = material.occlusionTexture;
    const tinygltf::TextureInfo& emissive = material.emissiveTexture;
    
    
    uint64 hash = ((uint64)(glm::max(pbr.baseColorTexture.index, 0) & 0xFFF)) 
                | ((uint64)(glm::max(pbr.metallicRoughnessTexture.index, 0) & 0xFFF) << 12) 
                | ((uint64)(glm::max(normal.index, 0) & 0xFFF) << 24) 
                | ((uint64)(glm::max(occlusion.index, 0) & 0xFFF) << 36) 
                | ((uint64)(glm::max(emissive.index, 0) & 0xFFF) << 48);
    if (hash && m_mtlMap.count(hash) > 0){
        return m_mtlMap[hash];
    }
    
    std::vector<uint32> texIndices;
    // base color
    uint32 bc_textureIndex = 0;
    vec4 baseColorFactor = {1.f,1.f,1.f,1.f};
    if (pbr.baseColorTexture.index >= 0){ // base color
        outMaterialFlags |= 0b1;
        bc_textureIndex = handleTexture(pbr.baseColorTexture, SPR_TEXTURE_COLOR);
    }

    // metallicroughness
    uint32 mr_textureIndex = 0;
    float metalFactor = 1.0f;
    float roughFactor = 1.0f;
    if (pbr.metallicRoughnessTexture.index >= 0){ //metallicroughness
        outMaterialFlags |= (0b1<<1);
        mr_textureIndex = handleTexture(pbr.metallicRoughnessTexture, SPR_TEXTURE_MR);
    }

    // normal
    uint32 n_textureIndex = 0;
    float normalScale = 1.0f;
    if (normal.index >= 0){
        outMaterialFlags |= (0b1<<2);
        n_textureIndex = handleTexture(normal);
        normalScale = (float)normal.scale;
    }

    // occlusion
    uint32 o_textureIndex = 0;
    float occlusionStrength = 1.0f;
    if (occlusion.index >= 0){
        outMaterialFlags |= (0b1<<3);
        o_textureIndex = handleTexture(occlusion);
    }

    // emissive
    uint32 e_textureIndex = 0;
    vec3 emissiveFactor = {0.f,0.f,0.f};
    if (emissive.index >= 0){
        outMaterialFlags |= (0b1<<4);
        e_textureIndex = handleTexture(emissive, SPR_TEXTURE_EMISSIVE);
    }

    // alphamode
    std::string alpha = material.alphaMode;
    uint32 alphaType = 0;
    float alphaCutoff = material.alphaCutoff;
    if (alpha != "OPAQUE"){
        outMaterialFlags |= (0b1<<5);
        if (alpha == "MASK")
            alphaType = 1;
        else 
            alphaType = 2;
    }

    // doublesided 
    if (material.doubleSided){
        outMaterialFlags |= (0b1<<6);
    }

    // write material to file
    MaterialLayout materialWrite {
        .id = 0,
        .materialFlags = outMaterialFlags,
        .bc_textureIndex = bc_textureIndex,
        .baseColorFactor = baseColorFactor,

        .mr_textureIndex = mr_textureIndex,
        .metalFactor = metalFactor,
        .roughnessFactor = roughFactor,

        .n_textureIndex = n_textureIndex,
        .normalScale = normalScale,

        .o_textureIndex = o_textureIndex,
        .occlusionStrength = occlusionStrength,

        .e_textureIndex = e_textureIndex,
        .emissiveFactor = emissiveFactor,

        .alphaType = alphaType,
        .alphaCutoff = alphaCutoff
    };

    uint32 materialIndex = writeMaterialFile(materialWrite);
    m_mtlMap[hash] = materialIndex;
    return materialIndex;
}


OffsetSpan GLTFParser::interleaveVertexAttributes(
        uint32 vertexCount,
        std::vector<uint8_t>& normalBuffer,
        std::vector<uint8_t>& tangentBuffer,
        std::vector<uint8_t>& texCoordBuffer,
        std::vector<uint8_t>& colorBuffer,
        glm::mat4& transform,
        DataRegion region){

    if (normalBuffer.size() != vertexCount * BYTES_PER_NORMAL){
        normalBuffer.resize(vertexCount * BYTES_PER_NORMAL);
    }
    if (tangentBuffer.size() != vertexCount * BYTES_PER_TANGENT){
        tangentBuffer.resize(vertexCount * BYTES_PER_TANGENT);
    }
    if (texCoordBuffer.size() != vertexCount * BYTES_PER_TEXCOORD){
        texCoordBuffer.resize(vertexCount * BYTES_PER_TEXCOORD);
    }
    if (colorBuffer.size() != vertexCount * BYTES_PER_COLOR){
        colorBuffer.resize(vertexCount * BYTES_PER_COLOR);
        glm::vec3 defaultColor = {1.f, 1.f, 1.f};
        for (uint32 i = 0; i < colorBuffer.size(); i+= BYTES_PER_COLOR){
            memcpy(((unsigned char*)colorBuffer.data() + i), (unsigned char*)glm::value_ptr(defaultColor), BYTES_PER_COLOR);
        }
    }

    glm::mat3 modelMatrix = glm::mat3(transform);
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(modelMatrix));

    // transform normals
    glm::vec3 normal = glm::vec3(1.0);
    for (uint32 i = 0; i < vertexCount*BYTES_PER_NORMAL; i += BYTES_PER_NORMAL){
        normal = glm::make_vec3((float*)(normalBuffer.data() + i));
        normal = glm::normalize(normalMatrix * normal);
        memcpy((unsigned char*)(normalBuffer.data() + i), ((unsigned char*)glm::value_ptr(normal)), BYTES_PER_NORMAL);
    }

    // transform tangents
    glm::vec4 tangent = glm::vec4(1.0);
    glm::vec3 t = glm::vec3(1.0);
    float modelSign = glm::determinant(normalMatrix) < 0.0f ? -1.0f : 1.0f;
    for (uint32 i = 0; i < vertexCount*BYTES_PER_TANGENT; i += BYTES_PER_TANGENT){
        uint32 vertex = i / BYTES_PER_TANGENT;
        uint32 normalOffset = vertex * BYTES_PER_NORMAL;
        glm::vec3 n = glm::make_vec3((float*)(normalBuffer.data() + normalOffset));

        tangent = glm::make_vec4((float*)(tangentBuffer.data() + i));
        t = glm::normalize(normalMatrix * glm::vec3(tangent));
        t = glm::normalize(t - n * glm::dot(t, n));
        tangent = glm::vec4(t, tangent.w * modelSign);
        memcpy((unsigned char*)(tangentBuffer.data() + i), ((unsigned char*)glm::value_ptr(tangent)), BYTES_PER_TANGENT);
    }

    // interleave into attributes buffer
    std::vector<uint8_t> result(vertexCount*BYTES_PER_ATTRIBUTE);
    // for each vertex, manually copy into 'result' such that it takes the form:
    //
    //      [ vec3 | vec2.x ]    OR    [ normal | texCoord.U ]
    //      [ vec3 | vec2.y ]          [ color  | texCoord.V ]
    //      [ vec4          ]          [ tangent             ]
    
    for (uint32 vertex = 0; vertex < vertexCount; vertex++){
        uint32 offset = vertex*BYTES_PER_ATTRIBUTE;
        // copy normal 
        for(uint32 normal = 0; normal < BYTES_PER_NORMAL; normal++){
            result[offset + normal] = normalBuffer[vertex*BYTES_PER_NORMAL + normal];
        }
        offset += BYTES_PER_NORMAL;

        // copy texCoord.U
        for(uint32 tex = 0; tex < BYTES_PER_TEXCOORD/2; tex++){
            result[offset + tex] = texCoordBuffer[vertex*BYTES_PER_TEXCOORD + tex];
        }
        offset += BYTES_PER_TEXCOORD/2;

        // copy color
        for(uint32 color = 0; color < BYTES_PER_COLOR; color++){
            result[offset + color] = colorBuffer[vertex*BYTES_PER_COLOR + color];
        }
        offset += BYTES_PER_COLOR;
        
        // copy texCoord.V
        for(uint32 tex = 0; tex < BYTES_PER_TEXCOORD/2; tex++){
            result[offset + tex] = texCoordBuffer[vertex*BYTES_PER_TEXCOORD + tex + BYTES_PER_TEXCOORD/2];
        }
        offset += BYTES_PER_TEXCOORD/2;

        // copy tangent
        for(uint32 tangent = 0; tangent < BYTES_PER_TANGENT; tangent++){
            result[offset + tangent] = tangentBuffer[vertex*BYTES_PER_TANGENT + tangent];
        }
        offset += BYTES_PER_TANGENT;
        
    }

    // write to buffer
    return writeBufferFile(result.data(), vertexCount*BYTES_PER_ATTRIBUTE, region);
}


void buildTangents(
            uint32 vertexCount,
            std::vector<uint8_t>& indicesBuffer,
            std::vector<uint8_t>& positionsBuffer,
            std::vector<uint8_t>& texCoordsBuffer,
            std::vector<uint8_t>& normalsBuffer,
            std::vector<uint8_t>& tangentsBuffer){
        
        uint32* indices = ((uint32*)indicesBuffer.data());
        glm::vec4* positions = ((glm::vec4*)positionsBuffer.data());

        glm::vec2* texCoords = ((glm::vec2*)texCoordsBuffer.data());
        glm::vec3* normals = ((glm::vec3*)normalsBuffer.data());

        tangentsBuffer.resize(vertexCount * BYTES_PER_TANGENT);
        glm::vec4* tangents = ((glm::vec4*)tangentsBuffer.data());

        glm::vec3 tangentTemp[vertexCount];
        glm::vec3 bitangentTemp[vertexCount];

        uint32 indexCount = indicesBuffer.size() / BYTES_PER_INDEX;
        for (uint32 i = 0; i < indexCount; i+= 3){
            // pull indices
            uint32 index0 = indices[i];
            uint32 index1 = indices[i+1];
            uint32 index2 = indices[i+2];
            
            // pull positions
            glm::vec4& v0 = positions[index0];
            glm::vec4& v1 = positions[index1];
            glm::vec4& v2 = positions[index2];

            // pull tex coords
            glm::vec2& uv0 = texCoords[index0];
            glm::vec2& uv1 = texCoords[index1];
            glm::vec2& uv2 = texCoords[index2];


            glm::vec3 dp1 = v1-v0;
            glm::vec3 dp2 = v2-v0;

            glm::vec2 duv1 = uv1-uv0;
            glm::vec2 duv2 = uv2-uv0;

            float r = 1.0f / (duv1.x * duv2.y - duv1.y * duv2.x);
            glm::vec3 tangent = (dp1 * duv2.y  - dp2 * duv1.y)*r;
            glm::vec3 bitangent = (dp2 * duv1.x  - dp1 * duv2.x)*r;

            //float handedness = (glm::dot(glm::cross(N, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;
            tangentTemp[index0] = tangent;
            tangentTemp[index1] = tangent;
            tangentTemp[index2] = tangent;

            bitangentTemp[index0] = bitangent;
            bitangentTemp[index1] = bitangent;
            bitangentTemp[index2] = bitangent;
        }

        for (uint32 i = 0; i < vertexCount; i++){
            glm::vec3 n = normals[i];
            glm::vec3 t = tangentTemp[i];
            glm::vec3 b = bitangentTemp[i];

            // Gram-Schmidt orthogonalize
            //t = glm::normalize(t - n * glm::dot(n, t));
            //float w = (glm::dot(glm::cross(n, t), b) < 0.0f) ? -1.0f : 1.0f;
            glm::vec4 t_out = {t,1.f};

            // write out
            tangents[i] = t_out;
        }
    }


OffsetSpan GLTFParser::handleAccessor(const tinygltf::Accessor& accessor, std::vector<uint8_t>& out, bool writeToFile, BufferData dataType, glm::mat4& transform, DataRegion region){
    // properties
    uint32 byteOffset = accessor.byteOffset;
    uint32 elementCount = accessor.count;
    uint32 elementType = accessor.type;
    uint32 componentType = accessor.componentType;
    uint32 bytesPerElement = tinygltf::GetNumComponentsInType(elementType) * tinygltf::GetComponentSizeInBytes(componentType);

    // buffer view
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

    // handle bufer view
    return handleBufferView(bufferView, std::string("sbuf"), byteOffset, bytesPerElement, elementCount, elementType, componentType, out, writeToFile, dataType, transform, region);
}


uint32 GLTFParser::handlePrimitive(const tinygltf::Primitive& primitive, glm::mat4& transform){
    // material
    int32 materialIndexGLTF = primitive.material;

    // attributes (accessor indices)
    int32 indicesAccessorIndex = primitive.indices;
    int32 positionAccessorIndex = -1;
    int32 normalAccessorIndex = -1;
    int32 tangentAccessorIndex = -1;
    int32 texcoordAccessorIndex = -1;
    int32 colorAccessorIndex = -1;

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

    uint64 hash = ((uint64)(materialIndexGLTF & 0xFFF)) 
                | ((uint64)(glm::max(indicesAccessorIndex, 0) & 0xFFF) << 12) 
                | ((uint64)(glm::max(positionAccessorIndex, 0) & 0xFFF) << 24) 
                | ((uint64)(glm::max(normalAccessorIndex+tangentAccessorIndex+texcoordAccessorIndex+colorAccessorIndex, 0) & 0xFFFFFFF) << 36) ;
    if (m_meshMap.count(hash) > 0){
        return m_meshMap[hash];
    }

    // handle accessors
    // indices
    std::vector<uint8_t> outIndices(16);
    outIndices.reserve(24);
    OffsetSpan indicesOffset = handleAccessor(model.accessors[indicesAccessorIndex], outIndices, true, SPR_INDICES, transform, SPR_DR_INDEX);
    
    // position
    OffsetSpan positionOffset;
    uint32 vertexCount = 0;
    std::vector<uint8_t> outPosition;
    if (positionAccessorIndex >= 0){
        vertexCount = model.accessors[positionAccessorIndex].count;
        positionOffset = handleAccessor(model.accessors[positionAccessorIndex], outPosition, true, SPR_POSITION, transform, SPR_DR_POSITION);
    }

    // normal
    std::vector<uint8_t> outNormal;
    if (normalAccessorIndex >= 0){
        handleAccessor(model.accessors[normalAccessorIndex], outNormal, false, SPR_NORMALS, transform, SPR_DR_ATTRIBUTE);
    }

    // texcoords
    std::vector<uint8_t> outTexCoord;
    if (texcoordAccessorIndex >= 0) {
        handleAccessor(model.accessors[texcoordAccessorIndex], outTexCoord, false, SPR_UV, transform, SPR_DR_ATTRIBUTE);
    }

    // colors
    std::vector<uint8_t> outColor;
    if (colorAccessorIndex >= 0) {
        handleAccessor(model.accessors[colorAccessorIndex], outColor, false, SPR_COLOR, transform, SPR_DR_ATTRIBUTE);
    }

    // tangent
    std::vector<uint8_t> outTangent;
    if (tangentAccessorIndex >= 0){
        handleAccessor(model.accessors[tangentAccessorIndex], outTangent, false, SPR_TANGENTS, transform, SPR_DR_ATTRIBUTE);
    } else {
        buildTangents(vertexCount, outIndices, outPosition, outTexCoord, outNormal, outTangent);
    }


    OffsetSpan attributesOffset = interleaveVertexAttributes(vertexCount, outNormal, outTangent, outTexCoord, outColor, transform, SPR_DR_ATTRIBUTE);

    // handle material
    uint32 materialIndex = 0;
    uint32 materialFlags = 0;
    if (materialIndexGLTF >= 0){
        materialIndex = handleMaterial(model.materials[materialIndexGLTF], materialFlags);
    }

    // write prim (mesh) to file
    MeshLayout meshWrite {
        .id = 0,

        .materialIndex = materialIndex,
        .materialFlags = materialFlags,

        .indexDataSizeBytes = indicesOffset.sizeBytes,
        .indexDataOffset = indicesOffset.offset,
        .indexBufferId = 0,

        .positionDataSizeBytes = positionOffset.sizeBytes,
        .positionDataOffset = positionOffset.offset,
        .positionBufferId = 0,
    
        .attributeDataSizeBytes = attributesOffset.sizeBytes,
        .attributeDataOffset = attributesOffset.offset,
        .attributeBufferId = 0
    };

    

    uint32 meshIndex = writeMeshFile(meshWrite);
    m_meshMap[hash] = meshIndex;
    return meshIndex;
}

void GLTFParser::handleMesh(const tinygltf::Mesh& mesh, std::vector<uint32> &meshIds, glm::mat4& transform){
    for (int32 i = 0; i < mesh.primitives.size(); i++){
        const tinygltf::Primitive& primitive = mesh.primitives[i];
        if (primitive.mode == 4 || primitive.mode == -1){
            
            std::string progress = "("+std::to_string(m_meshCount+1)+"/"+std::to_string(glm::max(model.meshes.size(), mesh.primitives.size()))+")";
            SprLog::log({
                {"  processing ", {125,125,125}, msg::CARRIAGE}, 
                {progress + " ", {150,150,150}}, 
                {mesh.name.substr(0,glm::min(40, (int)mesh.name.size()))+"...", {216, 151, 60}, msg::HOLD}
            });
            m_meshName = mesh.name;
            
            uint32 meshIndex = handlePrimitive(primitive, transform);
            if (std::find(meshIds.begin(), meshIds.end(), meshIndex) == meshIds.end()) {
                meshIds.push_back(meshIndex);
            }
        }
    }
}

void GLTFParser::vectorToMat4(const std::vector<double>& src, glm::mat4& dst){
    float mat[16];
    for (uint32 i = 0; i < 16; i++){
        mat[i] = (float)src.at(i);
    }
    dst = glm::make_mat4(mat);
}   

void GLTFParser::vectorToQuat(const std::vector<double>& src, glm::quat& dst){
    float quat[4];
    for (uint32 i = 0; i < 4; i++){
        quat[i] = (float)src.at(i);
    }
    dst = glm::make_quat(quat);
}

void GLTFParser::vectorToVec3(const std::vector<double>& src, glm::vec3& dst){
    float vec[3];
    for (uint32 i = 0; i < 3; i++){
        vec[i] = (float)src.at(i);
    }
    dst = glm::make_vec3(vec);
}

void GLTFParser::parseNode(const tinygltf::Node& node, std::vector<uint32> &meshIds, glm::mat4& transform){
    // get node transform matrix
    glm::mat4 nodeTransform = glm::mat4(1.0f);
    if (node.matrix.size() != 0){
        vectorToMat4(node.matrix, nodeTransform);
        //glm::transpose(nodeTransform);
    } else if (node.translation.size() || node.rotation.size() || node.scale.size()) {
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
    for (int32 i = 0; i < node.children.size(); i++){
        const tinygltf::Node& child = model.nodes[node.children[i]];
        parseNode(child, meshIds, nodeTransform);
    }
}

void GLTFParser::parse(){
    init();
    // assume one scene
    const tinygltf::Scene& scene = model.scenes[0];
    std::vector<uint32> meshIds;
    meshIds.reserve(256);

    // glTF (-X right, +Y up, +Z forward) -> engine (+X forward, -Y right, +Z up)
    glm::mat4 gltfToEngineBasis(1.0f);
    gltfToEngineBasis[0] = glm::vec4(0.f,  1.f, 0.f, 0.f); // glTF +X -> engine +Y
    gltfToEngineBasis[1] = glm::vec4(0.f,  0.f, 1.f, 0.f); // glTF +Y -> engine +Z
    gltfToEngineBasis[2] = glm::vec4(1.f,  0.f, 0.f, 0.f); // glTF +Z -> engine +X
    gltfToEngineBasis[3] = glm::vec4(0.f,  0.f, 0.f, 1.f);

    // process top level nodes
    for (int32 i = 0; i < scene.nodes.size(); i++){
        const tinygltf::Node& currNode = model.nodes[scene.nodes[i]];
        if (currNode.mesh >= 0 || currNode.children.size() > 0)
            parseNode(currNode, meshIds, gltfToEngineBasis);
    }

    consolidate();
    cleanup();
}

void GLTFParser::init(){
    SprLog::log({{msg::RESET}});
    m_modelStream.open("../data/temp/" + m_name + "_model.stmp", std::ios::binary);
    m_meshStream.open("../data/temp/" + m_name + "_mesh.stmp", std::ios::binary);
    m_materialStream.open("../data/temp/" + m_name + "_mtl.stmp", std::ios::binary);
    m_textureStream.open("../data/temp/" + m_name + "_tex.stmp", std::ios::binary);
    m_indexDataStream.open("../data/temp/" + m_name + "_indx.stmp", std::ios::binary);
    m_positionDataStream.open("../data/temp/" + m_name + "_pos.stmp", std::ios::binary);
    m_attributeDataStream.open("../data/temp/" + m_name + "_attr.stmp", std::ios::binary);
    m_textureDataStream.open("../data/temp/" + m_name + "_tdata.stmp", std::ios::binary);

    // empty material
    uint32 out;
    handleMaterial({}, out);
}

void GLTFParser::consolidate(){
    // write model file
    std::string modelName = m_name;
    modelName.resize(32);
    
    ModelHeader modelHeader = {
        .id = 0,
        .meshCount = m_meshCount,
        .materialCount = m_materialCount,
        .materialBufferOffset = 0,
        .textureCount = m_textureCount,
        .textureBufferOffset = 0,
        .blobHeaderOffset = 0,
        .blobDataOffset = 0
    };
    for (uint32 i = 0; i < 32; i++){
        modelHeader.name[i] = modelName[i];
    }
    modelHeader.materialBufferOffset = modelHeader.meshBufferOffset + m_meshCount * sizeof(MeshLayout);
    modelHeader.textureBufferOffset = modelHeader.materialBufferOffset + m_materialCount * sizeof(MaterialLayout);
    modelHeader.blobHeaderOffset = modelHeader.textureBufferOffset + m_textureCount * sizeof(TextureLayout);
    modelHeader.blobDataOffset = modelHeader.blobHeaderOffset + sizeof(BlobHeader);
    m_modelStream.write((char*)&modelHeader, sizeof(ModelHeader));

    // fill out blob header
    BlobHeader blobHeader = {
        .sizeBytes = m_indicesOffset + m_positionOffset + m_attributesOffset + m_textureDataOffset,
        .blobDataOffset = modelHeader.blobDataOffset,
        .indexRegionSizeBytes = m_indicesOffset,
        .indexRegionOffset = modelHeader.blobDataOffset,
        .positionRegionSizeBytes = m_positionOffset,
        .positionRegionOffset = modelHeader.blobDataOffset + m_indicesOffset,
        .attributeRegionSizeBytes = m_attributesOffset,
        .attributeRegionOffset = modelHeader.blobDataOffset + m_indicesOffset + m_positionOffset,
        .textureRegionSizeBytes = m_textureDataOffset,
        .textureRegionOffset = modelHeader.blobDataOffset + m_indicesOffset + m_positionOffset + m_attributesOffset
    };

    m_modelStream.close();
    m_meshStream.close();
    m_materialStream.close();
    m_textureStream.close();
    m_indexDataStream.close();
    m_positionDataStream.close();
    m_attributeDataStream.close();
    m_textureDataStream.close();

    m_modelStreamI.open("../data/temp/" + m_name + "_model.stmp", std::ios::binary);
    m_meshStreamI.open("../data/temp/" + m_name + "_mesh.stmp", std::ios::binary);
    m_materialStreamI.open("../data/temp/" + m_name + "_mtl.stmp", std::ios::binary);
    m_textureStreamI.open("../data/temp/" + m_name + "_tex.stmp", std::ios::binary);
    m_indexDataStreamI.open("../data/temp/" + m_name + "_indx.stmp", std::ios::binary);
    m_positionDataStreamI.open("../data/temp/" + m_name + "_pos.stmp", std::ios::binary);
    m_attributeDataStreamI.open("../data/temp/" + m_name + "_attr.stmp", std::ios::binary);
    m_textureDataStreamI.open("../data/temp/" + m_name + "_tdata.stmp", std::ios::binary);
    m_outputStream.open("../data/assets/" + m_name + ".smdl", std::ios::binary);

    // concat all streams into final temp output
    m_outputStream << m_modelStreamI.rdbuf();
    m_outputStream << m_meshStreamI.rdbuf();
    m_outputStream << m_materialStreamI.rdbuf();
    m_outputStream << m_textureStreamI.rdbuf();
    m_outputStream.write((char*)&blobHeader, sizeof(BlobHeader));
    m_outputStream << m_indexDataStreamI.rdbuf();
    m_outputStream << m_positionDataStreamI.rdbuf();
    m_outputStream << m_attributeDataStreamI.rdbuf();
    m_outputStream << m_textureDataStreamI.rdbuf();
    m_outputStream.flush();

    m_modelStreamI.close();
    m_meshStreamI.close();
    m_materialStreamI.close();
    m_textureStreamI.close();
    m_indexDataStreamI.close();
    m_positionDataStreamI.close();
    m_attributeDataStreamI.close();
    m_textureDataStreamI.close();
    m_outputStream.close();
}

void GLTFParser::cleanup(){
    SprLog::log({{msg::RESET}});
    std::filesystem::remove("../data/temp/"+(m_name + "_model")+".stmp");
    std::filesystem::remove("../data/temp/"+(m_name) + "_mtl"+".stmp");
    std::filesystem::remove("../data/temp/"+(m_name + "_tex")+".stmp");
    std::filesystem::remove("../data/temp/"+(m_name + "_indx")+".stmp");
    std::filesystem::remove("../data/temp/"+(m_name + "_pos")+".stmp");
    std::filesystem::remove("../data/temp/"+(m_name + "_attr")+".stmp");
    std::filesystem::remove("../data/temp/"+(m_name + "_tdata")+".stmp");
    std::filesystem::remove("../data/temp/"+(m_name + "_mesh")+".stmp");
}

// parse .gltf file
void GLTFParser::parseJson(std::string path){
    SprLog::log({{"  reading .gltf", {125,125,125}, msg::CARRIAGE}, {msg::HOLD}});
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        SprLog::log({{"  warn: ", color::WARN}, {warn}});
    }

    if (!err.empty()) {
        SprLog::log({{"  error: ", color::ERROR}, {err}});
        return;
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
    SprLog::log({{"  reading .glb", {125,125,125}, msg::CARRIAGE}, {msg::HOLD}});
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        SprLog::log({{"  warn: ", color::WARN}, {warn}});
    }

    if (!err.empty()) {
        SprLog::log({{"  error: ", color::ERROR}, {err}});
        return;
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
