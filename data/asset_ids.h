#pragma once

#include "../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::data{
typedef enum {
    // === Null Resource ===
    null_resource = 0,

    // === Imported Resources ===
    // models:
    default_model = 14,
    triangle = 9,
    duck = 1,

    // textures:
    default_input_white = 24,
    default_color = 20,
    default_input_black = 22,

    // shaders:
    // audio:
} ResourceId;

class ResourceIds{
public:
    // Filename <-> ResourceId map
    ska::flat_hash_map<std::string, uint32_t> idMap = 
    {
        // models
        {"default_model", 14},
        {"Triangle", 9},
        {"Duck", 1},
        // textures
        {"default_input_white", 24},
        {"default_color", 20},
        {"default_input_black", 22},
        // sub-resources
        {"Duck_6", 8},
        {"Duck_2", 6},
        {"Duck_1", 2},
        {"default_model_2", 17},
        {"default_input_black_1", 23},
        {"Triangle_1", 10},
        {"default_model_1", 15},
        {"default_model_3", 18},
        {"Duck_7", 3},
        {"Triangle_3", 12},
        {"Triangle_4", 13},
        {"Duck_9", 5},
        {"Duck_8", 4},
        {"Duck_3", 7},
        {"default_color_1", 21},
        {"default_model_6", 19},
        {"default_input_white_1", 25},
        {"default_model_7", 16},
        {"Triangle_2", 11},
    };

    uint32_t getIdFromName(std::string name){
        return idMap[name];
    }
};
}
