#pragma once

#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include "../core/Core.h"

namespace spr {

typedef enum : uint32 {
    SPR_NONE,
    SPR_MESH,
    SPR_MODEL,
    SPR_AUDIO,
    SPR_SHADER,
    SPR_BUFFER,
    SPR_TEXTURE,
    SPR_MATERIAL
} ResourceType;

std::string extensions[] = {
    ".snon",
    ".smsh",
    ".smdl",
    ".saud",
    ".shdr",
    ".sbuf",
    ".stex",
    ".smtl"
};

std::string paths[] = {
    "../data/none/",
    "../data/meshes/",
    "../data/models/",
    "../data/audio/",
    "../data/shaders/",
    "../data/buffers/",
    "../data/textures/",
    "../data/materials/",
};


typedef struct {
    std::string name = "no-name";         // resource name (file name)
    ResourceType resourceType = SPR_NONE; // resource enum (links path/ext)
    uint32 sizeBytes = 0;                 // TODO: self size, or umbrella?
    uint32 resourceId = 0;                // resource-unique id (enum value)
} ResourceMetadata;

typedef struct : ResourceMetadata {
    uint32 meshCount;
    std::vector<uint32> meshIds;
} ModelMetadata;

typedef struct : ResourceMetadata {
    uint32 materialId;
    int32 indexBufferId;
    int32 positionBufferId;
    int32 normalBufferId;
    int32 colorBufferId;
    int32 tangentBufferId;
    std::vector<int32> texCoordBufferIds;
} MeshMetadata;

typedef struct : ResourceMetadata {
    uint32 materialFlags;

    uint32 baseColorTexId;
    glm::vec4 baseColorFactor;

    uint32 metalRoughTexId;
    float metalFactor;
    float roughnessFactor;

    uint32 normalTexId;
    float normalScale;

    uint32 occlusionTexId;
    float occlusionStrength;

    uint32 emissiveTexId;
    glm::vec3 emissiveFactor;

    uint32 alphaType;
    uint32 alphaCutoff;

    bool doubleSided;
} MaterialMetadata;

typedef struct : ResourceMetadata {
    uint32 bufferId;
    uint32 imageType;
} TextureMetadata;

typedef struct : ResourceMetadata {
    uint32 elementType;
    uint32 componentType;
    uint32 byteLength;
} BufferMetadata;

typedef struct { 
    uint32 id;                  // instance id
} ResourceInstance;  


typedef struct : ResourceInstance {

} Model;


typedef struct : ResourceInstance {

} Mesh;


typedef struct : ResourceInstance {

} Material;


typedef struct : ResourceInstance {
    
} Texture;


typedef struct : ResourceInstance {

} Buffer;


typedef struct : ResourceInstance {

} Audio;


typedef struct : ResourceInstance {

} Shader;

class ResourceTypes {
public:
    static std::string getExtension(ResourceType resourceType){
        return extensions[resourceType];
    }

    static std::string getPath(ResourceType resourceType){
        return paths[resourceType];
    }
};

}