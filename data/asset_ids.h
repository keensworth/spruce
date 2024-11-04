#pragma once

#include "../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::data{
typedef enum {
    // === Null Resource ===
    null_resource = 0,

    // === Imported Resources ===
    // models:
    boomboxwithaxes = 7,
    default_model = 11,
    cube = 5,
    sponza = 10,
    helmet = 9,
    bistro = 4,
    triangle = 6,
    duck = 3,
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
        {"default_model", 11},
        {"Cube", 5},
        {"sponza", 10},
        {"helmet", 9},
        {"bistro", 4},
        {"Triangle", 6},
        {"Duck", 3},
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
