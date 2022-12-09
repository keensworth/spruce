#pragma once

#include "../external/flat_hash_map/flat_hash_map.hpp"


namespace spr{
typedef enum {
    // === Null Resource ===
    null_resource = 0,

    // === Imported Resources ===
    // models:
    default_model = 20,
    box = 14,
    triangle = 10,
    duck = 1,
    // shaders:
    // audio:
} ResourceId;

class ResourceIds{
public:
    // Filename <-> ResourceId map
    ska::flat_hash_map<std::string, uint32_t> idMap = 
    {
        // models
        {"default_model", 20},
        {"Box", 14},
        {"Triangle", 10},
        {"Duck", 1},
        // sub-resources
        {"Duck_6", 3},
        {"Duck_2", 6},
        {"Duck_1", 2},
        {"default_model_2", 23},
        {"Triangle_1", 11},
        {"default_model_1", 21},
        {"Duck_5", 9},
        {"default_model_3", 24},
        {"Duck_7", 4},
        {"Box_1", 15},
        {"Triangle_3", 13},
        {"default_model_4", 25},
        {"Box_4", 19},
        {"Duck_8", 5},
        {"default_model_5", 26},
        {"Duck_3", 7},
        {"default_model_6", 22},
        {"Box_3", 18},
        {"Box_5", 16},
        {"Triangle_2", 12},
        {"Duck_4", 8},
        {"Box_2", 17},
    };

    uint32_t getIdFromName(std::string name){
        return idMap[name];
    }
};
}
