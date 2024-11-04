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
    uint32 rootId = 0;
    uint32 id     = 0;
};  

// model
struct Model {
    ResourceInstance info;
    std::vector<uint32> meshIds;
};


// mesh
struct Mesh {
    ResourceInstance info;
    uint32 materialId         = 0;
    uint32 indexBufferId      = 0;
    uint32 positionBufferId   = 0;
    uint32 attributesBufferId = 0;
    uint32 materialFlags      = 0;
    uint32 pad0;
};


// material
struct Material {
    typedef enum : uint32 {
        BASE_COLOR         = 1,
        METALLIC_ROUGHNESS = 1<<1,
        NORMAL             = 1<<2,
        OCCLUSION          = 1<<3,
        EMISSIVE           = 1<<4,
        ALPHA              = 1<<5,
        DOUBLE_SIDED       = 1<<6,
        UNLIT              = 1<<10,
        WIREFRAME          = 1<<11,
        RECEIVES_SHADOWS   = 1<<12,
        CASTS_SHADOWS      = 1<<13,
        REFLECTIVE         = 1<<14,
        ALL                = 0xFFFFFFFF,
        NONE               = 0x00000000
    } Flags;

    ResourceInstance info;

    uint32 materialFlags = NONE;

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

    float alphaCutoff = 0.5f;
};


// texture
struct Texture {
    ResourceInstance info;

    uint32 bufferId   = 0;
    uint32 height     = 0;
    uint32 width      = 0;
    uint32 components = 0;

    uint32 pad0;
    uint32 pad1;
};


// buffer
struct Buffer {
    ~Buffer(){
        
    }
    ResourceInstance info;

    uint32 byteLength = 0;
    uint32 byteOffset = 0;
    spr::Span<uint8> data;
};

// unused
typedef struct Audio {} Audio;
typedef struct Shader {} Shader;


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

    uint32 id;

    // offset of xxx buffer (in bytes)
    // relative to .smdl file
    uint32 meshCount;
    static const uint32 meshBufferOffset = 64;

    uint32 materialCount;
    uint32 materialBufferOffset;

    uint32 textureCount;
    uint32 textureBufferOffset;

    uint32 blobHeaderOffset;
    uint32 blobDataOffset;
};

struct MeshLayout {
    uint32 id;

    // index of mesh's material in
    // .smdl Material buffer
    uint32 materialIndex;
    uint32 materialFlags;

    // offset of xxx data (in bytes) relative
    // to start of xxx region in blob, where a 
    // region is a collection of same-type buffers
    uint32 indexDataSizeBytes;
    uint32 indexDataOffset;
    uint32 indexBufferId;

    uint32 positionDataSizeBytes;
    uint32 positionDataOffset;
    uint32 positionBufferId;
    
    uint32 attributeDataSizeBytes;
    uint32 attributeDataOffset;
    uint32 attributeBufferId;
};

struct MaterialLayout {
    uint32 id;

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
};

struct TextureLayout {
    uint32 id;

    // offset of texture data (in bytes)
    // relative to start of Texture region
    uint32 dataSizeBytes;
    uint32 dataOffset;
    uint32 dataBufferId;

    uint32 height;
    uint32 width;
    uint32 components;

    uint32 pad0;
};

struct BlobHeader {
    // size of all data in blob
    uint32 sizeBytes;
    
    // offsets from start of .smdl
    uint32 blobDataOffset;

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
};

}