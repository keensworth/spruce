#include "ResourceLoader.h"
#include "debug/SprLog.h"
#include "ResourceTypes.h"
#include "SprResourceManager.h"
#include <cstring>

namespace spr{

ResourceLoader::ResourceLoader(){

}

template <typename T>
void ResourceLoader::loadFromMetadata(MetadataMap& metadataMap, ResourceMetadata& metadata, T& data){
    SprLog::error("[ResourceLoader] Unkown resource");
}

bool ResourceLoader::checkMapping(uint32 id){
    if (!m_mmap.is_mapped()){
        m_mmap.map(m_pathMap[id], m_error);
        m_mappedId = id;
    }

    if (m_mappedId != id){
        m_mmap.unmap();
        m_mmap.map(m_pathMap[id], m_error);
        m_mappedId = id;
    }

    if (m_error){
        SprLog::warn("[ResourceLoader] Failed to open asset file at "
         + m_pathMap[id] + " with id: ", id);
         disable();
         return false;
    }

    return true;
}

void ResourceLoader::disable(){
    if (m_mmap.is_mapped()){
        m_mmap.unmap();
    }
}


// ------------------------------------------------------------------------- //
//    Model - .smdl                                                          // 
// ------------------------------------------------------------------------- //
template <>
void ResourceLoader::loadFromMetadata<Model>(MetadataMap& metadataMap, ResourceMetadata& metadata, Model& modelOut){
    if (!checkMapping(metadata.parentId)){
        return;
    }

    // traverse model file for the target model
    ModelHeader& model = ((ModelHeader*)(m_mmap.data() + 0))[0];
    spr::Span<MeshLayout> meshes = {(MeshLayout*) (m_mmap.data() + model.meshBufferOffset), model.meshCount};

    // create model, write to output
    modelOut = {
        .info = { 
            .rootId = metadata.parentId, 
            .id     = metadata.resourceId 
        },
        .meshIds = std::vector<uint32>()
    };
    modelOut.meshIds.reserve(model.meshCount);
    
    // update model's mesh ids
    uint32 i = 0;
    for (const MeshLayout& mesh : meshes){
        modelOut.meshIds.push_back(mesh.id);

        // create child metadata if it doesn't exist
        if (metadataMap.count(mesh.id)){
            continue;
        }

        metadataMap[mesh.id] = {
            .resourceType = SPR_MESH,
            .resourceId = mesh.id,
            .parentId = metadata.parentId,
            .sizeTotal  = mesh.attributeDataSizeBytes
                        + mesh.positionDataSizeBytes
                        + mesh.indexDataSizeBytes,
            .byteOffset = model.meshBufferOffset,
            .byteLength = 0,
            .index = i++
        };
    }
}


// ------------------------------------------------------------------------- //
//    Mesh - .smsh                                                           // 
// ------------------------------------------------------------------------- //
template <>
void ResourceLoader::loadFromMetadata<Mesh>(MetadataMap& metadataMap, ResourceMetadata& metadata, Mesh& meshOut){
    if (!checkMapping(metadata.parentId)){
        return;
    }
    
    // traverse model file for the target mesh
    ModelHeader& model = ((ModelHeader*)(m_mmap.data() + 0))[0];
    MeshLayout& mesh = ((MeshLayout*)(m_mmap.data() + model.meshBufferOffset))[metadata.index];    
    MaterialLayout& material = ((MaterialLayout*)(m_mmap.data() + model.materialBufferOffset))[mesh.materialIndex];

    // create mesh, write to output
    meshOut = {
        .info = { 
            .rootId = metadata.parentId, 
            .id     = metadata.resourceId 
        },
        .materialId         = material.id,
        .indexBufferId      = mesh.indexBufferId,
        .positionBufferId   = mesh.positionBufferId,
        .attributesBufferId = mesh.attributeBufferId,
        .materialFlags      = material.materialFlags
    };

    // create child metadata if it doesn't exist
    if (metadataMap.count(material.id) 
     && metadataMap.count(mesh.indexBufferId) 
     && metadataMap.count(mesh.positionBufferId) 
     && metadataMap.count(mesh.attributeBufferId)){
        return;
     }

    metadataMap[material.id] = {
        .resourceType = SPR_MATERIAL,
        .resourceId = material.id,
        .parentId = metadata.parentId,
        .byteOffset = model.materialBufferOffset,
        .byteLength = 0,
        .index = mesh.materialIndex
    };
    
    BlobHeader& blob = ((BlobHeader*)(m_mmap.data() + model.blobHeaderOffset))[0];

    metadataMap[mesh.indexBufferId] = {
        .resourceType = SPR_BUFFER,
        .resourceId = mesh.indexBufferId,
        .parentId = metadata.parentId,
        .byteOffset = blob.indexRegionOffset + mesh.indexDataOffset,
        .byteLength = mesh.indexDataSizeBytes,
        .index = 0
    };

    metadataMap[mesh.positionBufferId] = {
        .resourceType = SPR_BUFFER,
        .resourceId = mesh.positionBufferId,
        .parentId = metadata.parentId,
        .byteOffset = blob.positionRegionOffset + mesh.positionDataOffset,
        .byteLength = mesh.positionDataSizeBytes,
        .index = 0
    };

    metadataMap[mesh.attributeBufferId] = {
        .resourceType = SPR_BUFFER,
        .resourceId = mesh.attributeBufferId,
        .parentId = metadata.parentId,
        .byteOffset = blob.attributeRegionOffset + mesh.attributeDataOffset,
        .byteLength = mesh.attributeDataSizeBytes,
        .index = 0
    };   
}


// ------------------------------------------------------------------------- //
//    Material - .smtl                                                       // 
// ------------------------------------------------------------------------- //
template <>
void ResourceLoader::loadFromMetadata<Material>(MetadataMap& metadataMap, ResourceMetadata& metadata, Material& materialOut){
    if (!checkMapping(metadata.parentId)){
        return;
    }

    // traverse model file for the target material
    ModelHeader& model = ((ModelHeader*)(m_mmap.data() + 0))[0];
    MaterialLayout& material = ((MaterialLayout*)(m_mmap.data() + model.materialBufferOffset))[metadata.index];
    TextureLayout* textures = (TextureLayout*)(m_mmap.data() + model.textureBufferOffset);
    uint32 flags = material.materialFlags;

    uint32 baseColorTexId = (flags & Material::BASE_COLOR) ? textures[material.bc_textureIndex].id : 0;
    uint32 metalRoughTexId = (flags & Material::METALLIC_ROUGHNESS) ? textures[material.mr_textureIndex].id : 0;
    uint32 normalTexId = (flags & Material::NORMAL) ? textures[material.n_textureIndex].id : 0;
    uint32 occlusionTexId = (flags & Material::OCCLUSION) ? textures[material.o_textureIndex].id : 0;
    uint32 emissiveTexId = (flags & Material::EMISSIVE) ? textures[material.e_textureIndex].id : 0;

    // create material, write to output
    materialOut = {
        .info = { 
            .rootId = metadata.parentId, 
            .id     = metadata.resourceId 
        },
        .materialFlags = material.materialFlags,

        .baseColorTexId  = baseColorTexId,
        .baseColorFactor = {material.baseColorFactor},

        .metalRoughTexId = metalRoughTexId,
        .metalFactor     = material.metalFactor, 
        .roughnessFactor = material.roughnessFactor, 

        .normalTexId = normalTexId,
        .normalScale = material.normalScale, 

        .occlusionTexId    = occlusionTexId,
        .occlusionStrength = material.occlusionStrength, 

        .emissiveTexId  = emissiveTexId,
        .emissiveFactor = {material.emissiveFactor}, 

        .alphaCutoff = material.alphaCutoff
    };

    // create child metadata if it doesn't exist
    if (flags & Material::BASE_COLOR && !metadataMap.count(baseColorTexId)){
        metadataMap[baseColorTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = baseColorTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = model.textureBufferOffset,
            .byteLength = 0,
            .index = material.bc_textureIndex
        };       
    }
    if (flags & Material::METALLIC_ROUGHNESS && !metadataMap.count(metalRoughTexId)){
        metadataMap[metalRoughTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = metalRoughTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = model.textureBufferOffset,
            .byteLength = 0,
            .index = material.mr_textureIndex
        };
    }
    if (flags & Material::NORMAL && !metadataMap.count(normalTexId)){
        metadataMap[normalTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = normalTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = model.textureBufferOffset,
            .byteLength = 0,
            .index = material.n_textureIndex
        };
    }
    if (flags & Material::OCCLUSION && !metadataMap.count(occlusionTexId)){
        metadataMap[occlusionTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = occlusionTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = model.textureBufferOffset,
            .byteLength = 0,
            .index = material.o_textureIndex
        };
    }
    if (flags & Material::EMISSIVE && !metadataMap.count(emissiveTexId)){
        metadataMap[emissiveTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = emissiveTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = model.textureBufferOffset,
            .byteLength = 0,
            .index = material.e_textureIndex
        };
    }
}


// ------------------------------------------------------------------------- //
//    Texture - .stex                                                        // 
// ------------------------------------------------------------------------- //
template <>
void ResourceLoader::loadFromMetadata<Texture>(MetadataMap& metadataMap, ResourceMetadata& metadata, Texture& textureOut){
    if (!checkMapping(metadata.parentId)){
        return;
    }
    
    // traverse model file for the target texture
    ModelHeader& model = ((ModelHeader*)(m_mmap.data()))[0];
    uint32 layoutOffset = metadata.sub ? model.textureBufferOffset : 0;
    TextureLayout& texture = ((TextureLayout*)(m_mmap.data() + layoutOffset))[metadata.index];

    // create texture, write to output
    textureOut = {
        .info = { 
            .rootId = metadata.parentId, 
            .id     = metadata.resourceId 
        },
        .bufferId   = texture.dataBufferId,
        .height     = texture.height,
        .width      = texture.width,
        .components = texture.components
    };

    // create child metadata if it doesn't exist
    if (metadataMap.count(texture.dataBufferId)){
        return;
    }

    BlobHeader& blob = ((BlobHeader*)(m_mmap.data() + model.blobHeaderOffset))[0];
    uint32 offset = metadata.sub ? blob.textureRegionOffset + texture.dataOffset : sizeof(TextureLayout);
    metadataMap[texture.dataBufferId] = {
        .resourceType = SPR_BUFFER,
        .resourceId = texture.dataBufferId,
        .parentId = metadata.parentId,
        .byteOffset = offset,
        .byteLength = texture.dataSizeBytes,
        .index = 0,
        .sub = metadata.sub
    };
}


// ------------------------------------------------------------------------- //
//    Buffer - .sbuf                                                         // 
// ------------------------------------------------------------------------- //
template <>
void ResourceLoader::loadFromMetadata<Buffer>(MetadataMap& metadataMap, ResourceMetadata& metadata, Buffer& bufferOut){
    if (!checkMapping(metadata.parentId)){
        return;
    }

    bufferOut = {
        .info = { 
            .rootId = metadata.parentId, 
            .id     = metadata.resourceId 
        },
        .byteLength = metadata.byteLength,
        .byteOffset = metadata.byteOffset,
        .data = {(uint8*)m_mmap.data() + metadata.byteOffset, metadata.byteLength}
    };
}

// ----------------------------------------------------------------------------
//    Audio - .***
//    iw
template <>
void ResourceLoader::loadFromMetadata<Audio>(MetadataMap& metadataMap, ResourceMetadata& metadata, Audio& audio){
}


// ----------------------------------------------------------------------------
//    Shader - .glsl
//    iw
template <>
void ResourceLoader::loadFromMetadata<Shader>(MetadataMap& metadataMap, ResourceMetadata& metadata, Shader& shader){
}

}