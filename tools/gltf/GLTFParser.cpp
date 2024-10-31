#include <cstring>
#include <fstream>
#include "GLTFParser.h"
#include <stdio.h>
#include "Resources.h"
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
    bool needsPosPadding = false;
    if (dataType == SPR_POSITION){
        assert (elementType == TINYGLTF_TYPE_VEC3 || elementType == TINYGLTF_TYPE_VEC4);
        if (elementType == TINYGLTF_TYPE_VEC3){
            byteLength = (4.f/3.f)*byteLength;
            elementType = TINYGLTF_TYPE_VEC4;
            needsPosPadding = true;
        }
    }

    bool needsIndicesPadding = false;
    if (dataType == SPR_INDICES){
        if (componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT){
            byteLength = 2*byteLength;
            elementType = TINYGLTF_TYPE_SCALAR;
            componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
            needsIndicesPadding = true;
        }
    }

    // write slice of buffer into new buffer
    const unsigned char* bufferData = buffer.data.data();
    unsigned char* data = new unsigned char[byteLength];
    if (!needsPosPadding) {
        if (!needsIndicesPadding){
            for (int32 i = 0; i < byteLength; i++){
                data[i] = bufferData[i+byteOffset];
            }
        } else {
            uint32 index = 0;
            for (int32 i = 0; i < byteLength; i++){
                if (i%4 >= 2){ 
                    data[i] = 0;
                } else {
                    data[i] = bufferData[index+byteOffset];
                    index++;
                }
            }
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
        uint32 index = 0;
        for (int32 i = 0; i < byteLength; i++){
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
        for (uint32 i = 0; i < byteLength; i += 16){
            glm::vec4 pos = glm::make_vec4((float*)(data + i));
            pos = transform * pos;
            memcpy((unsigned char*)(data + i), ((unsigned char*)glm::value_ptr(pos)), 16);
        }
    }

    OffsetSpan offsetSpan {0, 0};
    if (writeToFile){
        // write slice to file
        offsetSpan = writeBufferFile(data, byteLength, region);
    } else {
        // write slice to 'out'
        out.resize(byteLength);
        memcpy(out.data(), data, byteLength);
    }

    delete[] data;

    return offsetSpan;
}



void GLTFParser::createMip(
        unsigned char* in, 
        uint32 inSizeBytes, 
        uint32 inExtent, 
        unsigned char* mipOut, 
        uint32 outSizeBytes, 
        uint32 outExtent)
{
    stbir_resize_uint8(in, inExtent, inExtent, 0, mipOut, outExtent, outExtent, 0, 4);
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
    createInfo.vkFormat = dataType == SPR_TEXTURE_COLOR ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
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
        std::cerr << "Failed to create KTX2 texture, code: " << ktxErrorString(result) << std::endl;
    }

    uint32 levels = createInfo.numLevels;
    uint32 maxExtent = std::max(width, height);

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
    unsigned char* data = new unsigned char[byteLength];
    for (int32 i = 0; i < byteLength; i++){
        data[i] = bufferData[i+byteOffset];
    }

    unsigned char* ktxTextureData = nullptr;
    uint32 ktxTextureDataSize;

    // generate mips + compress
    compressImageData(data, byteLength, &ktxTextureData, ktxTextureDataSize, dataType, width, height, 4);

    OffsetSpan offsetSpan = writeBufferFile(ktxTextureData, ktxTextureDataSize, SPR_DR_TEXTURE);

    free(ktxTextureData);
    delete[] data;
    
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
    for (int32 i = 0; i < byteLength; i++)
        data[i] = pixels[i];
    free(pixels);

    unsigned char* ktxTextureData = nullptr;
    uint32 ktxTextureDataSize;

    compressImageData(data, byteLength, &ktxTextureData, ktxTextureDataSize, dataType, width, height, 4); 

    OffsetSpan offsetSpan = writeBufferFile(ktxTextureData, ktxTextureDataSize, SPR_DR_TEXTURE);

    free(ktxTextureData);
    delete[] data;

    return offsetSpan;
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
        DataRegion region){
    std::vector<unsigned char> bufferData = buffer.data;
    std::vector<unsigned char> data;
    // iterate over buffer, one stride at a time 
    for (uint32 i = byteOffset; i < byteOffset + byteLength; i+= byteStride){
        // grab neccessary bytes from stride (byte-by byte, may be slow)
        for (uint32 b = 0; b < bytesPerElement; b++){
            data.push_back(bufferData[i+b]);
        }
    }

    if (writeToFile){
        // write data to file
        return writeBufferFile(data.data(), byteLength, region);
    } else {
        // write data to 'out'
        out.resize(byteLength);
        memcpy(out.data(), data.data(), byteLength);
    }
    return {0,0};
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
    //uint32 byteLength = bufferView.byteLength - byteOffset;
    uint32 byteLength = elementCount * bytesPerElement;
    uint32 byteStride = bufferView.byteStride;

    if (byteStride == 0)
        byteStride = bytesPerElement;

    // buffer
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    // handle buffer
    if (byteStride == bytesPerElement){
        if (!association.compare("sbuf")){ // normal case
            return handleBuffer(buffer, association, adjustedByteOffset, byteLength, bytesPerElement, elementCount, elementType, componentType, out, writeToFile, dataType, transform, region);
        }else{  // handle buffer that contains MIME image data
            return handleMIMEImageBuffer(buffer, association, adjustedByteOffset, byteLength, bytesPerElement, elementCount, elementType, componentType, dataType);
        }
    } else{
        return handleBufferInterleaved(buffer, association, adjustedByteOffset, byteLength, byteStride, bytesPerElement, elementType, componentType, out, writeToFile, region);
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

uint32 GLTFParser::handleTexture(const tinygltf::Texture& tex, BufferData dataType){
    std::vector<uint8_t> out;
    // get image and sampler
    int32 sourceIndex = tex.source;
    int32 samplerIndex = tex.sampler;
    int32 minFilter;
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
    //uint32 maskedTexId = (texId & 0xFFFF) | (minFilter << 16);

    // get image
    image = model.images[sourceIndex];

    // get image components
    uint32 components = image.component;
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
    int32 elementType = TINYGLTF_TYPE_VEC4;
    int32 componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    glm::mat4 temp;

    OffsetSpan textureOffset;
    if (image.bufferView >= 0){ // bufferview
        int32 elementCount = image.width * image.height * image.component;
        textureOffset = handleBufferView(model.bufferViews[image.bufferView], std::string("stex"), 0, 1, elementCount, elementType, componentType, out, true, dataType, temp, SPR_DR_TEXTURE);
    } else { // direct buffer
        tinygltf::Buffer buffer;
        int32 elementCount = image.image.size();
        int32 elementType = TINYGLTF_TYPE_SCALAR;
        int32 componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        buffer.data = image.image;
        uint32 bytesPerElement = tinygltf::GetNumComponentsInType(elementType) * tinygltf::GetComponentSizeInBytes(componentType);
        textureOffset = handleTextureBuffer(buffer, std::string("stex"), 0, bytesPerElement*elementCount, bytesPerElement, elementCount, elementType, componentType, out, true, dataType, image.width, image.height, components);
    }
    m_sourceTexIdMap[sourceIndex] = m_textureIndex;
    m_sourceBuffIdMap[sourceIndex] = sourceIndex;

    // write texture to file
    TextureLayout texture {
        textureOffset.sizeBytes, 
        textureOffset.offset, 
        (uint32)image.height, 
        (uint32)image.width, 
        components
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
    return handleTexture(tex, SPR_TEXTURE_OTHER);
}

uint32 GLTFParser::handleMaterial(const tinygltf::Material& material, uint32& outMaterialFlags){
    std::vector<uint32> texIndices;
    // base color
    uint32 bc_textureIndex = 0;
    vec4 baseColorFactor = {1.f,1.f,1.f,1.f};
    const tinygltf::PbrMetallicRoughness& pbr = material.pbrMetallicRoughness;
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
        mr_textureIndex = handleTexture(pbr.metallicRoughnessTexture, SPR_TEXTURE_OTHER);
    }

    // normal
    uint32 n_textureIndex = 0;
    float normalScale = 1.0f;
    const tinygltf::NormalTextureInfo& normal = material.normalTexture;
    if (normal.index >= 0){
        outMaterialFlags |= (0b1<<2);
        n_textureIndex = handleTexture(normal);
    }

    // occlusion
    uint32 o_textureIndex = 0;
    float occlusionStrength = 1.0f;
    const tinygltf::OcclusionTextureInfo& occlusion = material.occlusionTexture;
    if (occlusion.index >= 0){
        outMaterialFlags |= (0b1<<3);
        o_textureIndex = handleTexture(occlusion);
    }

    // emissive
    uint32 e_textureIndex = 0;
    vec3 emissiveFactor = {0.f,0.f,0.f};
    const tinygltf::TextureInfo& emissive = material.emissiveTexture;
    if (emissive.index >= 0){
        outMaterialFlags |= (0b1<<4);
        e_textureIndex = handleTexture(emissive, SPR_TEXTURE_OTHER);
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
    uint32 doubleSided = 0;
    if (material.doubleSided){
        outMaterialFlags |= (0b1<<6);
        doubleSided = 1;
    }

    // write material to file
    MaterialLayout materialWrite {
        outMaterialFlags,
        bc_textureIndex,
        baseColorFactor,

        mr_textureIndex,
        metalFactor,
        roughFactor,

        n_textureIndex,
        normalScale,

        o_textureIndex,
        occlusionStrength,

        e_textureIndex,
        emissiveFactor,

        alphaType,
        alphaCutoff,

        doubleSided
    };

    return writeMaterialFile(materialWrite);
}

OffsetSpan GLTFParser::interleaveVertexAttributes(
        uint32 vertexCount,
        std::vector<uint8_t>& normalBuffer,
        std::vector<uint8_t>& tangentBuffer,
        std::vector<uint8_t>& texCoordBuffer,
        std::vector<uint8_t>& colorBuffer,
        glm::mat4& transform,
        DataRegion region){
    uint32 bytesPerNormal = 12;
    uint32 bytesPerColor = 12;
    uint32 bytesPerTexCoord = 8;
    uint32 bytesPerVertex = bytesPerNormal + bytesPerColor + bytesPerTexCoord;

    if (normalBuffer.size() != vertexCount * bytesPerNormal){
        normalBuffer.resize(vertexCount * bytesPerNormal);
    }
    if (colorBuffer.size() != vertexCount * bytesPerColor){
        colorBuffer.resize(vertexCount * bytesPerColor);
        glm::vec3 defaultColor = {1.f, 1.f, 1.f};
        for (uint32 i = 0; i < colorBuffer.size(); i+= bytesPerColor){
            memcpy(((unsigned char*)colorBuffer.data() + i), (unsigned char*)glm::value_ptr(defaultColor), bytesPerColor);
        }
    }
    if (texCoordBuffer.size() != vertexCount * bytesPerTexCoord){
        texCoordBuffer.resize(vertexCount * bytesPerTexCoord);
    }

    // transform normals
    for (uint32 i = 0; i < vertexCount*bytesPerNormal; i += bytesPerNormal){
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
    
    for (uint32 vertex = 0; vertex < vertexCount; vertex++){
        uint32 offset = vertex*bytesPerVertex;
        // copy normal 
        for(uint32 normal = 0; normal < bytesPerNormal; normal++){
            result[offset + normal] = normalBuffer[vertex*bytesPerNormal + normal];
        }
        offset += bytesPerNormal;

        // copy texCoord.U
        for(uint32 tex = 0; tex < bytesPerTexCoord/2; tex++){
            result[offset + tex] = texCoordBuffer[vertex*bytesPerTexCoord + tex];
        }
        offset += bytesPerTexCoord/2;

        // copy color
        for(uint32 color = 0; color < bytesPerColor; color++){
            result[offset + color] = colorBuffer[vertex*bytesPerColor + color];
        }
        offset += bytesPerColor;
        
        // copy texCoord.V
        for(uint32 tex = 0; tex < bytesPerTexCoord/2; tex++){
            result[offset + tex] = texCoordBuffer[vertex*bytesPerTexCoord + tex + bytesPerTexCoord/2];
        }
        offset += bytesPerTexCoord/2;
        
    }

    // write to buffer
    return writeBufferFile(result.data(), vertexCount*bytesPerVertex, region);
}

uint32 GLTFParser::handlePrimitive(const tinygltf::Primitive& primitive, glm::mat4& transform){
    std::vector<uint8_t> tempOut;

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

    // handle accessors
    // indices
    OffsetSpan indicesOffset = handleAccessor(model.accessors[indicesAccessorIndex], tempOut, true, SPR_INDICES, transform, SPR_DR_INDEX);
    
    // position
    OffsetSpan positionOffset;
    uint32 vertexCount = 0;
    if (positionAccessorIndex >= 0){
        vertexCount = model.accessors[positionAccessorIndex].count;
        positionOffset = handleAccessor(model.accessors[positionAccessorIndex], tempOut, true, SPR_POSITION, transform, SPR_DR_POSITION);
    }

    // normal
    OffsetSpan normalOffset;
    std::vector<uint8_t> outNormal;
    if (normalAccessorIndex >= 0){
        normalOffset = handleAccessor(model.accessors[normalAccessorIndex], outNormal, false, SPR_NORMALS, transform, SPR_DR_ATTRIBUTE);
    }

    // tangent
    OffsetSpan tangentOffset;
    std::vector<uint8_t> outTangent;
    if (tangentAccessorIndex >= 0){
        tangentOffset = handleAccessor(model.accessors[tangentAccessorIndex], outTangent, false, SPR_TANGENTS, transform, SPR_DR_ATTRIBUTE);
    }

    // texcoords
    OffsetSpan texCoordOffset;
    std::vector<uint8_t> outTexCoord;
    if (texcoordAccessorIndex >= 0) {
        texCoordOffset = handleAccessor(model.accessors[texcoordAccessorIndex], outTexCoord, false, SPR_UV, transform, SPR_DR_ATTRIBUTE);
    }

    // colors
    OffsetSpan colorOffset;
    std::vector<uint8_t> outColor;
    if (colorAccessorIndex >= 0) {
        colorOffset = handleAccessor(model.accessors[colorAccessorIndex], outColor, false, SPR_COLOR, transform, SPR_DR_ATTRIBUTE);
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
        materialIndex,
        materialFlags,

        indicesOffset.sizeBytes,
        indicesOffset.offset,

        positionOffset.sizeBytes,
        positionOffset.offset,
    
        attributesOffset.sizeBytes,
        attributesOffset.offset
    };
    return writeMeshFile(meshWrite);
}

void GLTFParser::handleMesh(const tinygltf::Mesh& mesh, std::vector<uint32> &meshIds, glm::mat4& transform){
    for (int32 i = 0; i < mesh.primitives.size(); i++){
        const tinygltf::Primitive& primitive = mesh.primitives[i];
        if (primitive.mode == 4 || primitive.mode == -1){
            meshIds.push_back(handlePrimitive(primitive, transform));
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

    // process top level nodes
    for (int32 i = 0; i < scene.nodes.size(); i++){
        const tinygltf::Node& currNode = model.nodes[scene.nodes[i]];
        glm::mat4 identity = glm::mat4(1.0f);
        parseNode(currNode, meshIds, identity);
    }

    consolidate();
    cleanup();
}

void GLTFParser::init(){
    m_modelStream.open("../data/temp/" + m_name + "_model.stmp", std::ios::binary);
    m_meshStream.open("../data/temp/" + m_name + "_mesh.stmp", std::ios::binary);
    m_materialStream.open("../data/temp/" + m_name + "_mtl.stmp", std::ios::binary);
    m_textureStream.open("../data/temp/" + m_name + "_tex.stmp", std::ios::binary);
    m_indexDataStream.open("../data/temp/" + m_name + "_indx.stmp", std::ios::binary);
    m_positionDataStream.open("../data/temp/" + m_name + "_pos.stmp", std::ios::binary);
    m_attributeDataStream.open("../data/temp/" + m_name + "_attr.stmp", std::ios::binary);
    m_textureDataStream.open("../data/temp/" + m_name + "_tdata.stmp", std::ios::binary);
}

void GLTFParser::consolidate(){
    // write model file
    std::string modelName = m_name;
    modelName.resize(32);
    
    ModelHeader modelHeader = {
        .meshCount = m_meshCount,
        .meshBufferOffset = 0,
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
    modelHeader.meshBufferOffset = sizeof(ModelHeader);
    modelHeader.materialBufferOffset = modelHeader.meshBufferOffset + m_meshCount * sizeof(MeshLayout);
    modelHeader.textureBufferOffset = modelHeader.materialBufferOffset + m_materialCount * sizeof(MaterialLayout);
    modelHeader.blobHeaderOffset = modelHeader.textureBufferOffset + m_textureCount * sizeof(TextureLayout);
    modelHeader.blobDataOffset = modelHeader.blobHeaderOffset + sizeof(BlobHeader);
    m_modelStream.write((char*)&modelHeader, sizeof(ModelHeader));

    // fill out blob header
    BlobHeader blobHeader = {
        .sizeBytes = m_indicesOffset + m_positionOffset + m_attributesOffset + m_textureDataOffset,
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