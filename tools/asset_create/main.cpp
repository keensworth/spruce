#include <stdlib.h>
#include "AssetCreator.h"

// parses any of the following formats into .stex:
//      .png
//      .jpg
//      .bmp
//      .tga

using namespace spr::tools;

int main(int argc, char **argv){
    // verify only one paremeter
    if (argc != 2){
        std::cout << "Incorrect number of arguments" << std::endl;
        std::cout << "usage: ./assetcreate <path-to-image>" << std::endl;
        return -1;
    }

    // verify valid extension
    std::string ext = std::filesystem::path(argv[1]).extension();
    if (ext != ".png" && ext != ".jpg" && ext != ".bmp" && ext != ".jpeg" && ext != ".tga"){
        std::cout << "Extension not supported" << std::endl;
        std::cout << "Expected: .png .jpg .jpeg .bmp .tga" << std::endl;
        std::cout << "Provided: " << ext << std::endl;
        return -1;
    }

    std::string filename(argv[1]);

    // parse file
    AssetCreator creator = AssetCreator();
    creator.createTexture(filename);
}