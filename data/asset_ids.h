#pragma once

#include "../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::data{
typedef enum {
    // === Null Resource ===
    null_resource = 0,

    // === Imported Resources ===
    // models:
    boomboxwithaxes = 8530,
    default_model = 8596,
    cube = 8515,
    helmet = 8580,
    bistro = 11,
    triangle = 8525,
    duck = 3,
    waterbottle = 8566,

    // textures:
    default_input_white = 8608,
    default_color = 8604,
    px = 8602,
    default_input_black = 8606,

    // shaders:
    // audio:
} ResourceId;

class ResourceIds{
public:
    // Filename <-> ResourceId map
    ska::flat_hash_map<std::string, uint32_t> idMap = 
    {
        // models
        {"BoomBoxWithAxes", 8530},
        {"default_model", 8596},
        {"Cube", 8515},
        {"helmet", 8580},
        {"bistro", 11},
        {"Triangle", 8525},
        {"Duck", 3},
        {"WaterBottle", 8566},
        // textures
        {"default_input_white", 8608},
        {"default_color", 8604},
        {"px", 8602},
        {"default_input_black", 8606},
    };

    uint32_t getIdFromName(std::string name){
        return idMap[name];
    }
};
}
