#include <stdlib.h>
#include "GLTFParser.h"

// parses gltf file into:
//      .smdl - model file
//      .sbuf - buffer file
//      .stex - texture file
//      .smtl - material file
//      .smsh - mesh file
//
// each file references the guids of associated components
// everything decomposes into buffers

using namespace spr::tools;

int main(int argc, char **argv){
    // verify only one paremeter
    if (argc != 2){
        std::cout << "Incorrect number of arguments" << std::endl;
        std::cout << "usage: ./gltfparser <path-to-file>" << std::endl;
        return -1;
    }

    // verify valid extension
    std::string ext = std::filesystem::path(argv[1]).extension();
    if (ext != ".gltf" && ext != ".glb"){
        std::cout << "Extension not supported" << std::endl;
        std::cout << "Expected: .glb .gltf" << std::endl;
        std::cout << "Provided: " << ext << std::endl;
        return -1;
    }

    // parse file
    GLTFParser parser = GLTFParser();
    if (ext == ".gltf"){
        parser.parseJson(argv[1]);
    } else { // .glb
        parser.parseBinary(argv[1]);
    }
}