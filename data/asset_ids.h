#pragma once

#include "../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::data{
typedef enum {
    // === Null Resource ===
    null_resource = 0,

    // === Imported Resources ===
    // models:
    boomboxwithaxes = 6,
    default_model = 10,
    cube = 4,
    helmet = 8,
    newsponza_main_gltf_003 = 9,
    triangle = 5,
    duck = 3,
    waterbottle = 7,

    // textures:
    default_input_white = 14,
    default_color = 12,
    px = 11,
    default_input_black = 13,

    // shaders:
    // audio:
} ResourceId;

class ResourceIds{
public:
    // Filename <-> ResourceId map
    ska::flat_hash_map<std::string, uint32_t> idMap = 
    {
        // models
        {"BoomBoxWithAxes", 6},
        {"default_model", 10},
        {"Cube", 4},
        {"helmet", 8},
        {"NewSponza_Main_glTF_003", 9},
        {"Triangle", 5},
        {"Duck", 3},
        {"WaterBottle", 7},
        // textures
        {"default_input_white", 14},
        {"default_color", 12},
        {"px", 11},
        {"default_input_black", 13},
    };

    uint32_t getIdFromName(std::string name){
        return idMap[name];
    }
};
}
