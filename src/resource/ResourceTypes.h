#pragma once

#include <string>
#include <typeindex>
#include <typeinfo>
#include "../core/Core.h"

namespace spr {

typedef enum : uint32 {
    SPR_NONE,
    SPR_NESH,
    SPR_MODEL,
    SPR_AUDIO,
    SPR_SHADER,
    SPR_BUFFER,
    SPR_TEXTURE,
    SPR_MATERIAL
} ResourceType;

typedef struct {
    std::string path = "no-path";
    std::string name = "no-name";
    std::string fileType = "no-type";
    ResourceType resourceType = SPR_NONE;
    uint32 sizeBytes = 0;
    uint32 resourceId = 0;
} ResourceMetadata;

typedef struct {
    uint32 id;
} ResourceInstance;

typedef struct : ResourceInstance {

} Mesh;

typedef struct : ResourceInstance {
    
} Model;

typedef struct : ResourceInstance {

} Audio;

typedef struct : ResourceInstance {

} Shader;

typedef struct : ResourceInstance {

} Buffer;

typedef struct : ResourceInstance {
    
} Texture;

typedef struct : ResourceInstance {

} Material;








}