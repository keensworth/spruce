#pragma once

#include "../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::data{
typedef enum {
    // === Null Resource ===
    null_resource = 0,

    // === Imported Resources ===
    // models:
    boomboxwithaxes = 15,
    default_model = 733,
    sponza = 81,
    helmet = 65,
    triangle = 10,
    duck = 2,
    waterbottle = 51,

    // textures:
    default_input_white = 743,
    default_color = 739,
    default_input_black = 741,

    // shaders:
    // audio:
} ResourceId;

class ResourceIds{
public:
    // Filename <-> ResourceId map
    ska::flat_hash_map<std::string, uint32_t> idMap = 
    {
        // models
        {"BoomBoxWithAxes", 15},
        {"default_model", 733},
        {"sponza", 81},
        {"helmet", 65},
        {"Triangle", 10},
        {"Duck", 2},
        {"WaterBottle", 51},
        // textures
        {"default_input_white", 743},
        {"default_color", 739},
        {"default_input_black", 741},
    };

    uint32_t getIdFromName(std::string name){
        return idMap[name];
    }
};
}
