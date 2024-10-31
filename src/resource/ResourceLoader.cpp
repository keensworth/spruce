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

void ResourceLoader::checkMapping(uint32 id){
    if (!m_mmap.is_mapped()){
        m_mmap.map(m_pathMap[id], m_error);
        m_mappedId = id;
        return;
    }

    if (m_mappedId != id){
        m_mmap.unmap();
        m_mmap.map(m_pathMap[id], m_error);
        m_mappedId = id;
        return;
    }
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
void ResourceLoader::loadFromMetadata<Model>(MetadataMap& metadataMap, ResourceMetadata& metadata, Model& model){
    checkMapping(metadata.parentId);

    ModelHeader& modelHeader = ((ModelHeader*)(m_mmap.data() + 0))[0];
    spr::Span<MeshLayout> meshLayouts = {(MeshLayout*) (m_mmap.data() + modelHeader.meshBufferOffset), modelHeader.meshCount};
    
    std::vector<uint32> meshIds;
    meshIds.reserve(modelHeader.meshCount);
    uint32 meshId = 0;
    uint32 i = 0;
    for (const MeshLayout& mesh : meshLayouts){
        meshId = ++m_id;
        metadataMap[meshId] = {
            .resourceType = SPR_MESH,
            .resourceId = meshId,
            .parentId = metadata.parentId,
            .sizeTotal  = mesh.attributeDataSizeBytes
                        + mesh.positionDataSizeBytes
                        + mesh.indexDataSizeBytes,
            .byteOffset = modelHeader.meshBufferOffset,
            .byteLength = 0,
            .index = i++
        };
        meshIds.push_back(meshId);
    }

    model.parentId = metadata.parentId;
    model.resourceId = metadata.resourceId;
    model.meshCount = modelHeader.meshCount;
    model.meshIds = meshIds;
}


// ------------------------------------------------------------------------- //
//    Mesh - .smsh                                                           // 
// ------------------------------------------------------------------------- //
template <>
void ResourceLoader::loadFromMetadata<Mesh>(MetadataMap& metadataMap, ResourceMetadata& metadata, Mesh& mesh){
    checkMapping(metadata.parentId);
    
    ModelHeader& modelHeader = ((ModelHeader*)(m_mmap.data() + 0))[0];
    MeshLayout& meshLayout = ((MeshLayout*)(m_mmap.data() + modelHeader.meshBufferOffset))[metadata.index];    
    
    MaterialLayout& material = ((MaterialLayout*)(m_mmap.data() + modelHeader.materialBufferOffset))[meshLayout.materialIndex];
    uint32 materialFlags = material.materialFlags;
    uint32 materialId = ++m_id;
    metadataMap[materialId] = {
        .resourceType = SPR_MATERIAL,
        .resourceId = materialId,
        .parentId = metadata.parentId,
        .byteOffset = modelHeader.materialBufferOffset,
        .byteLength = 0,
        .index = meshLayout.materialIndex
    };

    BlobHeader& blob = ((BlobHeader*)(m_mmap.data() + modelHeader.blobHeaderOffset))[0];
    uint32 indexBufferId = ++m_id;
    metadataMap[indexBufferId] = {
        .resourceType = SPR_BUFFER,
        .resourceId = indexBufferId,
        .parentId = metadata.parentId,
        .byteOffset = blob.indexRegionOffset + meshLayout.indexDataOffset,
        .byteLength = meshLayout.indexDataSizeBytes,
        .index = 0
    };

    uint32 positionBufferId = ++m_id;
    metadataMap[positionBufferId] = {
        .resourceType = SPR_BUFFER,
        .resourceId = positionBufferId,
        .parentId = metadata.parentId,
        .byteOffset = blob.positionRegionOffset + meshLayout.positionDataOffset,
        .byteLength = meshLayout.positionDataSizeBytes,
        .index = 0
    };

    uint32 attributesBufferId = ++m_id;
    metadataMap[attributesBufferId] = {
        .resourceType = SPR_BUFFER,
        .resourceId = attributesBufferId,
        .parentId = metadata.parentId,
        .byteOffset = blob.attributeRegionOffset + meshLayout.attributeDataOffset,
        .byteLength = meshLayout.attributeDataSizeBytes,
        .index = 0
    };

    mesh.parentId = metadata.parentId;
    mesh.resourceId = metadata.resourceId;
    mesh.materialId = materialId;
    mesh.indexBufferId = indexBufferId;
    mesh.positionBufferId = positionBufferId;
    mesh.attributesBufferId = attributesBufferId;
    mesh.materialFlags = materialFlags;
}


// ------------------------------------------------------------------------- //
//    Material - .smtl                                                       // 
// ------------------------------------------------------------------------- //
template <>
void ResourceLoader::loadFromMetadata<Material>(MetadataMap& metadataMap, ResourceMetadata& metadata, Material& material){
    checkMapping(metadata.parentId);
    
    uint32 materialFlags = 0;
    uint32 baseColorTexId = 0;
    uint32 metalRoughTexId = 0;
    uint32 normalTexId = 0;
    uint32 occlusionTexId = 0;
    uint32 emissiveTexId = 0;

    ModelHeader& modelHeader = ((ModelHeader*)(m_mmap.data() + 0))[0];
    MaterialLayout& materialLayout = ((MaterialLayout*)(m_mmap.data() + modelHeader.materialBufferOffset))[metadata.index];
    materialFlags = materialLayout.materialFlags;

    material.parentId = metadata.parentId;
    material.resourceId = metadata.resourceId;
    material.materialFlags = materialFlags;

    if (materialFlags & 0b1){ // base color
        baseColorTexId = ++m_id;
        metadataMap[baseColorTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = baseColorTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = modelHeader.textureBufferOffset,
            .byteLength = 0,
            .index = materialLayout.bc_textureIndex
        };
        material.baseColorTexId = baseColorTexId;
        material.baseColorFactor = materialLayout.baseColorFactor;
    }
    if (materialFlags & (0b1<<1)){ // metallicroughness
        metalRoughTexId = ++m_id;
        metadataMap[metalRoughTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = metalRoughTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = modelHeader.textureBufferOffset,
            .byteLength = 0,
            .index = materialLayout.mr_textureIndex
        };
        material.metalRoughTexId = metalRoughTexId;
        material.metalFactor = materialLayout.metalFactor;
        material.roughnessFactor = materialLayout.roughnessFactor;
    }
    if (materialFlags & (0b1<<2)){ // normal
        normalTexId = ++m_id;
        metadataMap[normalTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = normalTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = modelHeader.textureBufferOffset,
            .byteLength = 0,
            .index = materialLayout.n_textureIndex
        };
        material.normalTexId = normalTexId;
        material.normalScale = materialLayout.normalScale;
    }
    if (materialFlags & (0b1<<3)){ // occlusion
        occlusionTexId = ++m_id;
        metadataMap[occlusionTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = occlusionTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = modelHeader.textureBufferOffset,
            .byteLength = 0,
            .index = materialLayout.o_textureIndex
        };
        material.occlusionTexId = occlusionTexId;
        material.occlusionStrength = materialLayout.occlusionStrength;
    }
    if (materialFlags & (0b1<<4)){ // emissive
        emissiveTexId = ++m_id;
        metadataMap[emissiveTexId] = {
            .resourceType = SPR_TEXTURE,
            .resourceId = emissiveTexId,
            .parentId = metadata.parentId,
            .sizeTotal = 0,
            .byteOffset = modelHeader.textureBufferOffset,
            .byteLength = 0,
            .index = materialLayout.e_textureIndex
        };
        material.emissiveTexId = emissiveTexId;
        material.emissiveFactor = materialLayout.emissiveFactor;
    }
    if (materialFlags & (0b1<<5)){ // alpha
        material.alphaType = materialLayout.alphaType;
        material.alphaCutoff = materialLayout.alphaCutoff;
    }
    if (materialFlags & (0b1<<6)){ // double-sided
        material.doubleSided = true;
    }
}


// ------------------------------------------------------------------------- //
//    Texture - .stex                                                        // 
// ------------------------------------------------------------------------- //
template <>
void ResourceLoader::loadFromMetadata<Texture>(MetadataMap& metadataMap, ResourceMetadata& metadata, Texture& texture){
    checkMapping(metadata.parentId);
    
    ModelHeader& modelHeader = ((ModelHeader*)(m_mmap.data()))[0];
    uint32 layoutOffset = 0;
    if (metadata.sub){
        layoutOffset = modelHeader.textureBufferOffset;
    }
    TextureLayout& textureLayout = ((TextureLayout*)(m_mmap.data() + layoutOffset))[metadata.index];

    uint32 bufferId = ++m_id;
    uint32 offset = 0;
    if (metadata.sub){
        BlobHeader& blob = ((BlobHeader*)(m_mmap.data() + modelHeader.blobHeaderOffset))[0];
        offset = blob.textureRegionOffset + textureLayout.dataOffset;
    } else {
        offset = sizeof(TextureLayout);
    }

    metadataMap[bufferId] = {
        .resourceType = SPR_BUFFER,
        .resourceId = bufferId,
        .parentId = metadata.parentId,
        .byteOffset = offset,
        .byteLength = textureLayout.dataSizeBytes,
        .index = 0,
        .sub = metadata.sub
    };

    texture.parentId = metadata.parentId;
    texture.resourceId = metadata.resourceId;
    texture.bufferId = bufferId;
    texture.height = textureLayout.height;
    texture.width = textureLayout.width;
    texture.components = textureLayout.components;
}


// ------------------------------------------------------------------------- //
//    Buffer - .sbuf                                                         // 
// ------------------------------------------------------------------------- //
template <>
void ResourceLoader::loadFromMetadata<Buffer>(MetadataMap& metadataMap, ResourceMetadata& metadata, Buffer& buffer){
    checkMapping(metadata.parentId);

    buffer.parentId = metadata.parentId;
    buffer.resourceId = metadata.resourceId;
    buffer.byteLength = metadata.byteLength;
    buffer.byteOffset = metadata.byteOffset;
    buffer.data = {(uint8*)m_mmap.data() + metadata.byteOffset, metadata.byteLength};
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