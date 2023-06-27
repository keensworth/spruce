#pragma once

#include "../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::data{
typedef enum {
    // === Null Resource ===
    null_resource = 0,

    // === Imported Resources ===
    // models:
    boomboxwithaxes = 16,
    default_model = 734,
    sponza = 82,
    helmet = 66,
    triangle = 11,
    duck = 3,
    waterbottle = 52,

    // textures:
    default_input_white = 746,
    default_color = 742,
    px = 740,
    default_input_black = 744,

    // shaders:
    // audio:
} ResourceId;

class ResourceIds{
public:
    // Filename <-> ResourceId map
    ska::flat_hash_map<std::string, uint32_t> idMap = 
    {
        // models
        {"BoomBoxWithAxes", 16},
        {"default_model", 734},
        {"sponza", 82},
        {"helmet", 66},
        {"Triangle", 11},
        {"Duck", 3},
        {"WaterBottle", 52},
        // textures
        {"default_input_white", 746},
        {"default_color", 742},
        {"px", 740},
        {"default_input_black", 744},
    };

    uint32_t getIdFromName(std::string name){
        return idMap[name];
    }
};
}
