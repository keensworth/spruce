#pragma once

#include "../../src/core/spruce_core.h"
using namespace glm;

namespace spr::tools {

// ╔═ MODEL (.smdl) ═══════════════════╗<─ .smdl begin
// ║    ModelHeader                    ║
// ╠═══ BUFFERS ═══════════════════════╣<─ meshBufferOffset
// ║                                   ║
// ║      MeshLayout[]                 ║ 
// ║                                   ║ 
// ╠───────────────────────────────────╣<─ materialBufferOffset
// ║                                   ║
// ║      MaterialLayout[]             ║ 
// ║                                   ║ 
// ╠───────────────────────────────────╣<─ textureBufferOffset
// ║                                   ║
// ║      TextureLayout[]              ║ 
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
    vec4 baseColorFactor;

    uint32 mr_textureIndex;
    float metalFactor;
    float roughnessFactor;

    uint32 n_textureIndex;
    float normalScale;

    uint32 o_textureIndex;
    float occlusionStrength;

    uint32 e_textureIndex;
    vec3 emissiveFactor;

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
