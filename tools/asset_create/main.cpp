#include <stdlib.h>
#include "AssetCreator.h"

// parses any of the following formats into .stex:
//      .png
//      .jpg
//      .bmp
//      .tga

using namespace spr::tools;

int main(int argc, char **argv){
    bool cubeMap = !std::string("-cube").compare(std::string(argv[1]));

    // verify parameters
    if (argc != 2 && !cubeMap){
        std::cout << "Incorrect number of arguments" << std::endl;
        std::cout << "usage: ./assetcreate <path-to-image>" << std::endl;
        std::cout << " (or)  ./assetcreate -cube <path-X+> <path-X-> <path-Y+> <path-Y-> <path-Z+> <path-Z->" << std::endl;
        return -1;
    }

    // verify valid extension
    uint32_t index = cubeMap ? 2 : 1; 
    std::string ext = std::filesystem::path(argv[index]).extension();
    if (ext != ".png" && ext != ".jpg" && ext != ".bmp" && ext != ".jpeg" && ext != ".tga"){
        std::cout << "Extension not supported" << std::endl;
        std::cout << "Expected: .png .jpg .jpeg .bmp .tga" << std::endl;
        std::cout << "Provided: " << ext << std::endl;
        return -1;
    }

    AssetCreator creator;

    if (cubeMap){
        std::string facePaths[6] = {
            argv[2], // x+
            argv[3], // x- 
            argv[4], // y+
            argv[5], // y-
            argv[6], // z+
            argv[7]  // z-
        };
        // std::string facePaths[6] = {
        //     argv[2], // x+
        //     argv[3], // x- 
        //     argv[6], // y+
        //     argv[7], // y-
        //     argv[4], // z+
        //     argv[5]  // z-
        // };
        creator.createCubemapTexture(facePaths);
    } else {
        std::string filename(argv[1]);
        creator.createTexture(filename);
    }

    
}