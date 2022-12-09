#pragma once

#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include "../core/spruce_core.h"

namespace spr {
    

// ---------------- Types and mappings ----------------------------------------

// ResourceType enum
typedef enum ResourceType : uint32 {
    SPR_NONE,
    SPR_MESH,
    SPR_MODEL,
    SPR_AUDIO,
    SPR_SHADER,
    SPR_BUFFER,
    SPR_TEXTURE,
    SPR_MATERIAL
} ResourceType;

// ext associated with types (indexed by ResourceType)
static std::vector<std::string> extensions{
    ".snon",
    ".smsh",
    ".smdl",
    ".saud",
    ".shdr",
    ".sbuf",
    ".stex",
    ".smtl"
};

// path associated with types (indexed by ResourceType)
static std::vector<std::string> paths{
    "../data/none/",
    "../data/meshes/",
    "../data/models/",
    "../data/audio/",
    "../data/shaders/",
    "../data/buffers/",
    "../data/textures/",
    "../data/materials/"
};

static std::vector<std::string> resourceTypeStrings{
    "SPR_NONE",
    "SPR_MESH",
    "SPR_MODEL",
    "SPR_AUDIO",
    "SPR_SHADER",
    "SPR_BUFFER",
    "SPR_TEXTURE",
    "SPR_MATERIAL",
};

// ---------------- Metadata --------------------------------------------------
typedef struct ResourceMetadata {
    std::string name = "no-name";         // resource name (file name)
    ResourceType resourceType = SPR_NONE; // resource enum (links path/ext)
    uint32 sizeBytes = 0;                 // umbrella size in bytes
    uint32 resourceId = 0;                // resource-unique id (enum value)
} ResourceMetadata;

// ---------------- Instance --------------------------------------------------
// base instance data
typedef struct ResourceInstance { 
    uint32 id;             // instance id
    uint32 resourceId = 0; // resource-unique id (enum value)
} ResourceInstance;  


// model
typedef struct Model : ResourceInstance {
    uint32 meshCount = 0;
    std::vector<uint32> meshIds;
} Model;


// mesh
typedef struct Mesh : ResourceInstance {
    uint32 materialId = 0;
    int32 indexBufferId = -1;
    int32 positionBufferId = -1;
    int32 normalBufferId = -1;
    int32 colorBufferId = -1;
    int32 tangentBufferId = -1;
    std::vector<int32> texCoordBufferIds;
} Mesh;


// material
typedef struct Material : ResourceInstance {
    uint32 materialFlags = 0;

    int32 baseColorTexId = -1;
    glm::vec4 baseColorFactor = glm::vec4(1.f,1.f,1.f,1.f);

    int32 metalRoughTexId = -1;
    float metalFactor = 1;
    float roughnessFactor = 1;

    int32 normalTexId = -1;
    float normalScale = 1;

    int32 occlusionTexId = -1;
    float occlusionStrength = 1;

    int32 emissiveTexId = -1;
    glm::vec3 emissiveFactor = glm::vec3(0.f,0.f,0.f);

    int32 alphaType = -1;
    float alphaCutoff = 0.5f;

    bool doubleSided = false;
} Material;


// texture
typedef struct Texture : ResourceInstance {
    uint32 bufferId = -1;
    uint32 imageType = 0;
} Texture;


// buffer
typedef struct Buffer : ResourceInstance {
    uint32 elementType = 0;
    uint32 componentType = 0;
    uint32 byteLength = 0;
    char* data;
} Buffer;

// unused
typedef struct Audio : ResourceInstance {} Audio;
typedef struct Shader : ResourceInstance {} Shader;


// ---------------- Utility ---------------------------------------------------
class ResourceTypes {
public:
    
    static std::string getExtension(ResourceType resourceType){
        return extensions[resourceType];
    }

    static std::string getPath(ResourceType resourceType){
        return paths[resourceType];
    }
    
    static std::string typeToString(ResourceType resourceType){
        return resourceTypeStrings[resourceType];
    }

    static ResourceType stringToType(std::string type){
        if (type == "SPR_BUFFER")
            return SPR_BUFFER;
        else if (type == "SPR_MESH")
            return SPR_MESH;
        else if (type == "SPR_MATERIAL")
            return SPR_MATERIAL;
        else if (type == "SPR_TEXTURE")
            return SPR_TEXTURE;
        else if (type == "SPR_MODEL")
            return SPR_MODEL;
        else
            return SPR_NONE;
    }
};

}