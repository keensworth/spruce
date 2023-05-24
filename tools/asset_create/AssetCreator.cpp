#include "AssetCreator.h"
#include <cstdint>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


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

    writeBufferFile(data, x*y*4, elementType, componentType, 1);
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

}