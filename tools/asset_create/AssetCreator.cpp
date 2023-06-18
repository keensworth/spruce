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

    for (uint32_t i = 1; i < levels; i++){
        uint32_t currExtent = std::max(maxExtent / (1 << i), 1u);
        uint32_t currSize = currExtent*currExtent*components;
        level = i;
        layer = 0;
        faceSlice = 0;                           

        createMip(data, srcSize, maxExtent, mipData[i-1], currSize, currExtent);
        result = ktxTexture_SetImageFromMemory(ktxTexture(texture), level, layer, faceSlice, mipData[i-1], currSize);
        if (result) {
            std::cerr << "Failed to set (mip) image from memory, code: " << ktxErrorString(result) << std::endl;
        }
        
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

}