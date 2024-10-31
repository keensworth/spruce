#pragma once

#include "../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::data{
typedef enum {
    // === Null Resource ===
    null_resource = 0,

    // === Imported Resources ===
    // models:
    boomboxwithaxes = 27,
    default_model = 745,
    cube = 11,
    sponza = 93,
    helmet = 77,
    triangle = 21,
    duck = 3,
    waterbottle = 63,

    // textures:
    default_input_white = 761,
    default_color = 755,
    px = 752,
    default_input_black = 758,

    // shaders:
    // audio:
} ResourceId;

class ResourceIds{
public:
    // Filename <-> ResourceId map
    ska::flat_hash_map<std::string, uint32_t> idMap = 
    {
        // models
        {"BoomBoxWithAxes", 27},
        {"default_model", 745},
        {"Cube", 11},
        {"sponza", 93},
        {"helmet", 77},
        {"Triangle", 21},
        {"Duck", 3},
        {"WaterBottle", 63},
        // textures
        {"default_input_white", 761},
        {"default_color", 755},
        {"px", 752},
        {"default_input_black", 758},
    };

    uint32_t getIdFromName(std::string name){
        return idMap[name];
    }
};
}
