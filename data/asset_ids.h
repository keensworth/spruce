#pragma once

#include "../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::data{
typedef enum {
    // === Null Resource ===
    null_resource = 0,

    // === Imported Resources ===
    // models:
    boomboxwithaxes = 7,
    default_model = 10,
    cube = 5,
    damagedhelmet = 6,
    inferno = 11,
    newsponza_main_gltf_003 = 9,
    duck = 3,
    newsponza_curtains_gltf = 4,
    waterbottle = 8,

    // textures:
    default_input_white = 15,
    default_color = 13,
    px = 12,
    default_input_black = 14,

    // shaders:
    // audio:
} ResourceId;

class ResourceIds{
public:
    // Filename <-> ResourceId map
    ska::flat_hash_map<std::string, uint32_t> idMap = 
    {
        // models
        {"BoomBoxWithAxes", 7},
        {"default_model", 10},
        {"Cube", 5},
        {"DamagedHelmet", 6},
        {"inferno", 11},
        {"NewSponza_Main_glTF_003", 9},
        {"Duck", 3},
        {"NewSponza_Curtains_glTF", 4},
        {"WaterBottle", 8},
        // textures
        {"default_input_white", 15},
        {"default_color", 13},
        {"px", 12},
        {"default_input_black", 14},
    };

    uint32_t getIdFromName(std::string name){
        return idMap[name];
    }
};
}
