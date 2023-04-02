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
    unsigned char *data = stbi_load(path.c_str(), &x, &y, &n, 0);
    if (data == NULL){
        std::cerr << "Failed to open image" << std::endl;
    }

    uint32_t elementType = n;
    uint32_t componentType = 5121;

    writeBufferFile(data, x*y*n, elementType, componentType, 1);
    writeTextureFile(1, n, 0);

    stbi_image_free(data);
}

void AssetCreator::writeTextureFile(uint32_t bufferId, uint32_t components, uint32_t texId){
    // buffer id (4)
    // raw/png/jpg/raw/bmp (4)

    std::cout << "      Write Texture File:" << std::endl;

    // write to file
    // open file
    std::ofstream f("../data/textures/"+(m_name)+".stex", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }
    std::cout << "          f: created" << std::endl;
    
    // write buffer id
    f.write((char*)&bufferId, sizeof(uint32_t));
    std::cout << "          w: bufferId: " << bufferId << std::endl;

    // write image type
    f.write((char*)&components, sizeof(uint32_t));
    std::cout << "          w: components: " << components << std::endl;

    // close file
    f.close();
    std::cout << "          f: closed" << std::endl;
    std::cout << "" << std::endl;
}

void AssetCreator::writeBufferFile(const unsigned char* data, uint32_t byteLength, uint32_t elementType, uint32_t componentType, uint32_t bufferId){
    // element type (4)
    // component type (4)
    // byte length (4)
    // data (byte length)
    std::cout << "      Write Buffer File:" << std::endl;

    // write to file
    // open file
    std::ofstream f("../data/buffers/"+(m_name+("_"+std::to_string(bufferId)))+".sbuf", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }
    std::cout << "          f: created" << std::endl;

    // write element type
    f.write((char*)&elementType, sizeof(uint32_t));
    std::cout << "          w: elementType: " << elementType << std::endl;

    // write component type
    f.write((char*)&componentType, sizeof(uint32_t));
    std::cout << "          w: componentType: " << componentType << std::endl;

    // write byte length
    f.write((char*)&byteLength, sizeof(uint32_t));
    std::cout << "          w: byteLength: " << byteLength << std::endl;
    
    // write data 
    f.write((char*)data, byteLength);
    std::cout << "          w: data" << std::endl;

    // close file
    f.close();
    std::cout << "          f: closed" << std::endl;
    std::cout << "" << std::endl;
}

}