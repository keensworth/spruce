#pragma once

#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include "core/spruce_core.h"
#include "core/util/Span.h"

namespace spr {
    

// ---------------- Types and mappings ----------------------------------------

// ResourceType enum
enum ResourceType : uint32 {
    SPR_NONE,
    SPR_MESH,
    SPR_MODEL,
    SPR_AUDIO,
    SPR_SHADER,
    SPR_BUFFER,
    SPR_TEXTURE,
    SPR_MATERIAL
};

// ext associated with types (indexed by ResourceType)
static std::vector<std::string> extensions{
    ".snon",
    ".smdl",
    ".smdl",
    ".saud",
    ".shdr",
    ".smdl",
    ".smdl",
    ".smdl"
};

// path associated with types (indexed by ResourceType)
static std::vector<std::string> paths{
    "../data/none/",
    "../data/assets/",
    "../data/assets/",
    "../data/assets/",
    "../data/shaders/",
    "../data/assets/",
    "../data/assets/",
    "../data/assets/",
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

// --------------------------------------------------------- //
//                 Metadata                                  // 
// --------------------------------------------------------- //
struct ResourceMetadata {
    ResourceType resourceType = SPR_NONE;     
    uint32 resourceId = 0;  // resource-unique id (enum value)
    uint32 parentId = 0;
    uint32 sizeTotal = 0;
    uint32 byteOffset = 0;
    uint32 byteLength = 0;
    uint32 index = 0;
    uint32 sub = 1;
};



// --------------------------------------------------------- //
//                 Instance                                  // 
// --------------------------------------------------------- //
// base instance data
struct ResourceInstance { 
    uint32 parentId = 0;
    uint32 resourceId = 0;
};  


// model
struct Model : ResourceInstance {
    uint32 meshCount = 0;
    std::vector<uint32> meshIds;
};


// mesh
struct Mesh : ResourceInstance {
    uint32 materialId         = 0;
    uint32 indexBufferId      = 0;
    uint32 positionBufferId   = 0;
    uint32 attributesBufferId = 0;
    uint32 materialFlags      = 0;
};


// material
struct Material : ResourceInstance {
    uint32 materialFlags = 0;

    uint32 baseColorTexId     = 0;
    glm::vec4 baseColorFactor = glm::vec4(1.f,1.f,1.f,1.f);

    uint32 metalRoughTexId = 0;
    float metalFactor      = 1.0f;
    float roughnessFactor  = 1.0f;

    uint32 normalTexId = 0;
    float normalScale  = 1.0f;

    uint32 occlusionTexId   = 0;
    float occlusionStrength = 1.0f;

    uint32 emissiveTexId     = 0;
    glm::vec3 emissiveFactor = glm::vec3(0.f,0.f,0.f);

    uint32 alphaType  = 0;
    float alphaCutoff = 0.5f;

    bool doubleSided = false;
};


// texture
struct Texture : ResourceInstance {
    uint32 bufferId   = 0;
    uint32 height     = 0;
    uint32 width      = 0;
    uint32 components = 0;
};


// buffer
struct Buffer : ResourceInstance {
    ~Buffer(){
        
    }
    uint32 byteLength = 0;
    uint32 byteOffset = 0;
    spr::Span<uint8> data;
};

// unused
typedef struct Audio : ResourceInstance {} Audio;
typedef struct Shader : ResourceInstance {} Shader;


// --------------------------------------------------------- //
//                 Utility                                   // 
// --------------------------------------------------------- //
class ResourceTypes {
public:
    
    static std::string getExtension(ResourceType resourceType){
        return extensions[resourceType];
    }

    static std::string getExtension(ResourceType resourceType, uint32 sub){
        if (sub == 0 && (resourceType == SPR_TEXTURE || resourceType == SPR_BUFFER))
            return ".stex";

        return extensions[resourceType];
    }

    static std::string getPath(ResourceType resourceType){
        return paths[resourceType];
    }

    static std::string path(ResourceType resourceType, std::string name, uint32 sub){
        return getPath(resourceType)+name+getExtension(resourceType, sub);
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

// --------------------------------------------------------- //
//                 Disk Layout                               // 
// --------------------------------------------------------- //

// ╔═ MODEL (.smdl) ═══════════════════╗<─ .smdl begin
// ║    ModelHeader                    ║
// ╠═══ BUFFERS ═══════════════════════╣<─ meshBufferOffset
// ║                                   ║
// ║      Mesh[]                       ║ 
// ║                                   ║ 
// ╠───────────────────────────────────╣<─ materialBufferOffset
// ║                                   ║
// ║      Material[]                   ║ 
// ║                                   ║ 
// ╠───────────────────────────────────╣<─ textureBufferOffset
// ║                                   ║
// ║      Texture[]                    ║ 
// ║                                   ║ 
// ╠═══ BLOB ══════════════════════════╣<─ blobHeaderOffset
// ║      BlobHeader                   ║ 
// ╠═════ DATA REGIONS ════════════════╣<─ blobDataOffset
// ║                                   ║   indexRegionOffset
// ║        Index region (uint8[])     ║
// ║                                   ║ 
// ╠───────────────────────────────────╣<─ positionRegionOffset
// ║                                   ║ 
// ║        Position region (uint8[])  ║ 
// ║                                   ║ 
// ╠───────────────────────────────────╣<─ attributeRegionOffset
// ║                                   ║
// ║        Attribute region (uint8[]) ║
// ║                                   ║
// ╠───────────────────────────────────╣<─ textureRegionOffset
// ║                                   ║
// ║        Texture region (uint8[])   ║
// ║                                   ║
// ╚═══════════════════════════════════╝

// ╔═ BUFFER / Foo[] ══════════════════╗<─ fooBufferOffset
// ║                .                  ║   │
// ║───────────────────────────────────║<──┤ fooIndex
// ║    Foo                            ║   │    
// ║───────────────────────────────────║<──┘ fooIndex + 1
// ║    Foo                            ║ 
// ║───────────────────────────────────║ 
// ║                .                  ║ 
// ║                .                  ║ 
// ╚═══════════════════════════════════╝

// ╔═ Bar REGION ══════════════════════╗<─ barRegionOffset
// ║                .                  ║   │
// ║─── Bar DATA ──────────────────────║<──┤ + barDataOffset
// ║                                   ║   │    
// ║      uint8[barDataSizeBytes]      ║   │
// ║                                   ║   │
// ║───────────────────────────────────║<──┘ + barDataSizeBytes
// ║                .                  ║ 
// ║                .                  ║ 
// ╚═══════════════════════════════════╝

//
struct ModelHeader {
    char name[32];

    // offset of xxx buffer (in bytes)
    // relative to .smdl file
    uint32 meshCount;
    uint32 meshBufferOffset;

    uint32 materialCount;
    uint32 materialBufferOffset;

    uint32 textureCount;
    uint32 textureBufferOffset;

    uint32 blobHeaderOffset;
    uint32 blobDataOffset;
};

struct MeshLayout {
    // index of mesh's material in
    // .smdl Material buffer
    uint32 materialIndex;
    uint32 materialFlags;

    // offset of xxx data (in bytes) relative
    // to start of xxx region in blob, where a 
    // region is a collection of same-type buffers
    uint32 indexDataSizeBytes;
    uint32 indexDataOffset;

    uint32 positionDataSizeBytes;
    uint32 positionDataOffset;
    
    uint32 attributeDataSizeBytes;
    uint32 attributeDataOffset;
};

struct MaterialLayout {
    uint32 materialFlags;
    
    // index of mtl texture
    // in .smdl Texture buffer
    uint32 bc_textureIndex;
    glm::vec4 baseColorFactor;

    uint32 mr_textureIndex;
    float metalFactor;
    float roughnessFactor;

    uint32 n_textureIndex;
    float normalScale;

    uint32 o_textureIndex;
    float occlusionStrength;

    uint32 e_textureIndex;
    glm::vec3 emissiveFactor;

    uint32 alphaType;
    float alphaCutoff;

    uint32 doubleSided;
};

struct TextureLayout {
    // offset of texture data (in bytes)
    // relative to start of Texture region
    uint32 dataSizeBytes;
    uint32 dataOffset;

    uint32 height;
    uint32 width;
    uint32 components;

    uint32 pad0;
    uint32 pad1;
    uint32 pad2;
};

struct BlobHeader {
    // size of all data in blob
    uint32 sizeBytes;

    // offset from start of .smdl
    uint32 indexRegionSizeBytes;
    uint32 indexRegionOffset;

    uint32 positionRegionSizeBytes;
    uint32 positionRegionOffset;

    uint32 attributeRegionSizeBytes;
    uint32 attributeRegionOffset;

    uint32 textureRegionSizeBytes;
    uint32 textureRegionOffset;

    uint32 pad0;
    uint32 pad1;
    uint32 pad2;
};

}