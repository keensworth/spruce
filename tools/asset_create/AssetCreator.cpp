#include "AssetCreator.h"
#include <cstdint>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize.h"

#include "ktx.h"
#include "../../external/ktx/lib/vk_format.h"


namespace spr::tools {

AssetCreator::AssetCreator(){

}

AssetCreator::~AssetCreator(){

}

void AssetCreator::createTexture(std::string path){
    m_path = std::filesystem::path(path).parent_path().concat("/");
    m_name = std::filesystem::path(path).stem();
    m_extension = std::filesystem::path(path).extension();

    int x,y,n;
    unsigned char *data = stbi_load(path.c_str(), &x, &y, &n, STBI_rgb_alpha);
    if (data == NULL){
        std::cerr << "Failed to open image" << std::endl;
    }

    uint32_t elementType = 4;
    uint32_t componentType = 5121;

    unsigned char* ktxTextureData = nullptr;
    uint32_t ktxTextureDataSize;

    // generate mips + compress
    compressImageData(data, sizeof(data), &ktxTextureData, ktxTextureDataSize, SPR_TEXTURE_COLOR, x, y, 4);

    writeBufferFile(ktxTextureData, ktxTextureDataSize, elementType, componentType, 1);

    writeTextureFile(1, y, x, 4, 0);

    stbi_image_free(data);
}

void AssetCreator::createCubemapTexture(std::string facePaths[6]){
    // just using x+ name for result
    m_path = std::filesystem::path(facePaths[0]).parent_path().concat("/");
    m_name = std::filesystem::path(facePaths[0]).stem();
    m_extension = std::filesystem::path(facePaths[0]).extension();

    // load image data
    unsigned char* faceData[6];
    uint32_t width = 0;
    uint32_t height = 0;
    for (uint32_t i = 0; i < 6; i++){
        int x,y,n;
        faceData[i] = stbi_load(facePaths[i].c_str(), &x, &y, &n, STBI_rgb_alpha);
        if (faceData[i] == NULL){
            std::cerr << "Failed to open image, face: " << i << std::endl;
        }
        if (i > 0 && (x != width || y != height)){
            std::cerr << "Size mismatch, all faces must be same dimesion, face: " << i << std::endl;
        }
        width = x;
        height = y;
    }

    uint32_t elementType = 4;
    uint32_t componentType = 5121;

    unsigned char* ktxTextureData = nullptr;
    uint32_t ktxTextureDataSize;

    // generate mips + compress
    layerImageData(faceData, height*width*4, &ktxTextureData, ktxTextureDataSize, SPR_TEXTURE_COLOR, width, height, 4);

    writeBufferFile(ktxTextureData, ktxTextureDataSize, elementType, componentType, 1);

    writeTextureFile(1, height, width, 4, 0);

    for (uint32_t i = 0; i < 6; i++){
        stbi_image_free(faceData[i]);
    }
}

void AssetCreator::writeTextureFile(uint32_t bufferId, uint32_t height, uint32_t width, uint32_t components, uint32_t texId){
    // buffer id (4)
    // height (4)
    // width (4)
    // components (4)


    // write to file
    // open file
    std::ofstream f("../data/textures/"+(m_name)+".stex", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }
    
    // write buffer id
    f.write((char*)&bufferId, sizeof(uint32_t));

    // write height
    f.write((char*)&height, sizeof(uint32_t));

    // write width
    f.write((char*)&width, sizeof(uint32_t));

    // write components
    f.write((char*)&components, sizeof(uint32_t));

    // close file
    f.close();
}

void AssetCreator::writeBufferFile(const unsigned char* data, uint32_t byteLength, uint32_t elementType, uint32_t componentType, uint32_t bufferId){
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
    f.write("stex", sizeof(uint32_t));

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

void AssetCreator::compressImageData(
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

void AssetCreator::createMip(
        unsigned char* in, 
        uint32_t inSizeBytes, 
        uint32_t inExtent, 
        unsigned char* mipOut, 
        uint32_t outSizeBytes, 
        uint32_t outExtent)
{
    stbir_resize_uint8(in, inExtent, inExtent, 0, mipOut, outExtent, outExtent, 0, 4);
}


void AssetCreator::layerImageData(
        unsigned char* data[6],
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
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 6;
    createInfo.isArray = KTX_FALSE;
    createInfo.generateMipmaps = KTX_FALSE;
    
    result = ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
    if (result) {
        std::cerr << "Failed to create KTX2 texture, code: " << ktxErrorString(result) << std::endl;
    }

    // base level
    srcSize = width*height*components;
    level = 0;
    layer = 0;
    faceSlice = 0;                           

    for (uint32_t i = 0; i < 6; i++){
        level = 0;
        layer = 0;
        faceSlice = i;

        result = ktxTexture_SetImageFromMemory(ktxTexture(texture), level, layer, faceSlice, data[i], srcSize);
        if (result) {
            std::cerr << "Failed to set cube face image from memory, code: " << ktxErrorString(result) << std::endl;
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
    outDataSize = (uint32_t)outSize;
    ktxTexture_Destroy(ktxTexture(texture));
}

}