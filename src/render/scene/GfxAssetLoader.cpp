#include "GfxAssetLoader.h"
#include "Material.h"
#include "Mesh.h"
#include "resource/SprResourceManager.h"
#include "debug/SprLog.h"
#include "vulkan/TextureTranscoder.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include <string>

namespace spr::gfx {

GfxAssetLoader::GfxAssetLoader() {}

GfxAssetLoader::~GfxAssetLoader(){
    if (!m_cleared)
        clear();
}

MeshInfoMap GfxAssetLoader::loadAssets(SprResourceManager& rm, VulkanResourceManager* vrm, VulkanDevice* device){
    m_vertexPositions = {vrm, 4000000*sizeof(VertexPosition)};
    m_vertexAttributes = {vrm, 4000000*sizeof(VertexAttributes)};
    m_vertexIndices = {vrm, 4000000*sizeof(uint32)};
    m_materials = {vrm, 4096*sizeof(MaterialData)};

    std::vector<uint32>& modelIds = rm.getModelIds();
    std::vector<uint32>& textureIds = rm.getTextureIds();
    m_transcoder = TextureTranscoder(device);
    m_rm = vrm;

    MeshInfoMap map;

    // built-in assets
    loadBuiltinAssets(rm, map);

    // iterate over models and load subresources
    for (uint32 modelId : modelIds){
        Handle<spr::Model> modelHandle = rm.getHandle<spr::Model>(modelId);

        if (!modelHandle.isValid())
            SprLog::warn("[GfxAssetLoader] [loadAssets] invalid model");

        spr::Model* model = rm.getData<spr::Model>(modelHandle);

        // model's meshes
        std::vector<HandleID<Mesh>> meshHandleIds;
        for (uint32 meshId : model->meshIds){
            Handle<spr::Mesh> meshHandle = rm.getHandle<spr::Mesh>(meshId);
            meshHandleIds.push_back({meshHandle, meshId});
        }

        for (HandleID<Mesh> handleID : meshHandleIds){
            if (!handleID.handle.isValid())
                SprLog::warn("[GfxAssetLoader] [loadAssets] invalid mesh");

            spr::Mesh* mesh = rm.getData<spr::Mesh>(handleID.handle);

            MeshInfo meshInfo;
            loadVertexData(rm, mesh, meshInfo);
            loadMaterial(rm, mesh, meshInfo);

            map[handleID.id] = meshInfo;
        }
    }

    // load non-subresource textures
    for (uint32 texId : textureIds){
        loadTexture(rm, texId, true);
    }

    rm.disableLoader();

    return map;
}

void GfxAssetLoader::loadVertexData(SprResourceManager& rm, Mesh* mesh, MeshInfo& info){
    // indices
    if (mesh->indexBufferId && !m_indexBufferIds.count(mesh->indexBufferId)){
        Handle<spr::Buffer> indicesHandle = rm.getHandle<spr::Buffer>(mesh->indexBufferId);
        if (!indicesHandle.isValid()){
            SprLog::warn("[GfxAssetLoader] [loadVertexData] invalid indices");
            return;
        }

        spr::Buffer* indicesBuffer = rm.getData<spr::Buffer>(indicesHandle);
        auto alloc = m_vertexIndices.allocateAndInsert<uint32>({
            .data = indicesBuffer->data.data(), 
            .size = indicesBuffer->byteLength
        });

        info.indexCount = alloc.size;
        info.firstIndex = alloc.offset;
        m_counts.indexCount += info.indexCount;
        m_counts.bytes += alloc.byteSize;
        m_bufferHandles.push_back(indicesHandle);
        m_storedBuffersBytes += alloc.byteSize;
        m_indexBufferIds[mesh->indexBufferId] = 1;
    }

    // positions
    if (mesh->positionBufferId && !m_positionBufferIds.count(mesh->positionBufferId)){
        Handle<spr::Buffer> positionHandle = rm.getHandle<spr::Buffer>(mesh->positionBufferId);
        if (!positionHandle.isValid()){
            SprLog::warn("[GfxAssetLoader] [loadVertexData] invalid positions");
            return;
        }
        
        spr::Buffer* positionBuffer = rm.getData<spr::Buffer>(positionHandle);
        auto alloc = m_vertexPositions.allocateAndInsert<VertexPosition>({
            .data = positionBuffer->data.data(),
            .size = positionBuffer->byteLength
        });

        info.vertexOffset = alloc.offset;
        m_counts.vertexCount += alloc.size;
        m_counts.bytes += alloc.byteSize;
        m_bufferHandles.push_back(positionHandle);
        m_storedBuffersBytes += alloc.byteSize;
        m_positionBufferIds[mesh->positionBufferId] = 1;
    }

    // attributes
    if (mesh->attributesBufferId && !m_attributeBufferIds.count(mesh->attributesBufferId)){
        Handle<spr::Buffer> attributesHandle = rm.getHandle<spr::Buffer>(mesh->attributesBufferId);
        if (!attributesHandle.isValid()){
            SprLog::warn("[GfxAssetLoader] [loadVertexData] invalid attributes");
            return;
        }
        
        spr::Buffer* attributesBuffer = rm.getData<spr::Buffer>(attributesHandle);
        auto alloc = m_vertexAttributes.allocateAndInsert<VertexAttributes>({
            .data = attributesBuffer->data.data(),
            .size = attributesBuffer->byteLength
        });

        m_counts.bytes += alloc.byteSize;
        m_bufferHandles.push_back(attributesHandle);
        m_storedBuffersBytes += alloc.byteSize;
        m_attributeBufferIds[mesh->attributesBufferId] = 1;
    } 

    if (m_storedBuffersBytes >= MAX_STORED_BUFFER_BYTES){
        for (Handle<spr::Buffer> handle : m_bufferHandles){
            rm.deleteData(handle);
        }
        m_storedBuffersBytes = 0;
        m_bufferHandles.clear();
    }
}

void GfxAssetLoader::loadMaterial(SprResourceManager& rm, Mesh* mesh, MeshInfo& info){
    // process the mesh's material
    Handle<spr::Material> materialHandle = rm.getHandle<spr::Material>((mesh->materialId));
    if (!materialHandle.isValid()){
        SprLog::warn("[GfxAssetLoader] [loadMaterial] invalid material");
        return;
    }

    spr::Material* material = rm.getData(materialHandle);
    mesh->materialFlags = material->materialFlags;

    MaterialData materialData;

    if (material->baseColorTexId > 0){
        materialData.flags |= MTL_BASE_COLOR;
        materialData.baseColorTexIdx = loadTexture(rm, material->baseColorTexId, true);
        materialData.baseColorFactor = material->baseColorFactor;
    }
    if (material->metalRoughTexId > 0){
        materialData.flags |= MTL_METALLIC_ROUGHNESS;
        materialData.metalRoughTexIdx = loadTexture(rm, material->metalRoughTexId, false);
        materialData.metallicFactor = material->metalFactor;
    }
    if (material->normalTexId > 0){
        materialData.flags |= MTL_NORMAL;
        materialData.normalTexIdx = loadTexture(rm, material->normalTexId, false);
        materialData.normalScale = material->normalScale;
    }
    if (material->occlusionTexId > 0){
        materialData.flags |= MTL_OCCLUSION;
        materialData.occlusionTexIdx = loadTexture(rm, material->occlusionTexId, false);
        materialData.occlusionStrength = material->occlusionStrength;
    }
    if (material->emissiveTexId > 0){
        materialData.flags |= MTL_EMISSIVE;
        materialData.emissiveTexIdx = loadTexture(rm, material->emissiveTexId, false);
        materialData.emissiveFactor = material->emissiveFactor;
    }
    if (material->alphaType > 0){
        materialData.flags |= MTL_ALPHA;
        materialData.alphaCutoff = material->alphaCutoff;
    }
    if (material->doubleSided){
        materialData.flags |= MTL_DOUBLE_SIDED;
    }

    auto alloc = m_materials.allocateAndInsert<MaterialData>(materialData);
    
    info.materialIndex = alloc.offset;
    m_counts.materialCount++;
    m_counts.bytes += alloc.byteSize;
}

uint32 GfxAssetLoader::loadTexture(SprResourceManager& rm, uint32 textureId, bool srgb){
    // texture handle
    Handle<spr::Texture> handle = rm.getHandle<spr::Texture>(textureId);
    if (!handle.isValid()){
        SprLog::warn("[GfxAssetLoader] [loadTexture] invalid texture");
        return 0;
    }

    // texture data + buffer handle
    spr::Texture* texture = rm.getData<spr::Texture>(handle);
    Handle<spr::Buffer> texBufferHandle = rm.getHandle<spr::Buffer>(texture->bufferId);
    if (!texBufferHandle.isValid()){
        SprLog::warn("[GfxAssetLoader] [loadTexture] invalid buffer");
        return 0;
    }

    if (m_textureIds.count(texture->bufferId) > 0){
        return m_textureIds[texture->bufferId];
    }
    if (m_cubemapIds.count(texture->bufferId) > 0){
        return m_cubemapIds[texture->bufferId];
    }

    // texture buffer data
    spr::Buffer* texBuffer = rm.getData<spr::Buffer>(texBufferHandle);
    m_bufferHandles.push_back(texBufferHandle);
    m_storedBuffersBytes += texBuffer->byteLength;
       
    // transcode from ktx2 to relevant format // TODO!!
    TranscodeResult result; 
    m_transcoder.transcode(result, m_rm, (unsigned char*)texBuffer->data.data(), texBuffer->byteLength, texture->width, texture->height);
    result.transcodedData = {m_rm, result.sizeBytes};
    m_transcoder.destroyActiveTexture(result, result.sizeBytes);
    
    TextureInfo textureInfo = {
        .data = result.transcodedData,
        .height = texture->height,
        .width = texture->width,
        .components = texture->components,
        .format = result.format,
        .mipCount = result.mips,
        .layerCount = result.layers,
        .srgb = srgb
    };

    m_counts.bytes += result.sizeBytes;
    
    uint32 index = 0;
    if (result.layers == 6){
        index = m_cubemaps.size();
        m_cubemapIds[texture->bufferId] = index;

        m_cubemaps.push_back(textureInfo);
        m_counts.cubemapCount++;
    } else {
        index = m_textures.size();
        m_textureIds[texture->bufferId] = index;

        m_textures.push_back(textureInfo);
        m_counts.textureCount++;
    }

    m_transcoder.reset();

    if (m_storedBuffersBytes >= MAX_STORED_BUFFER_BYTES){
        for (Handle<spr::Buffer> handle : m_bufferHandles){
            rm.deleteData(handle);
        }
        m_storedBuffersBytes = 0;
        m_bufferHandles.clear();
    }

    return index;
}

void GfxAssetLoader::loadBuiltinAssets(SprResourceManager& rm, MeshInfoMap& meshes){
    // load built-in textures
    loadTexture(rm, spr::data::default_color, true);
    loadTexture(rm, spr::data::default_input_black, false);
    loadTexture(rm, spr::data::default_input_white, false);

    // load default/error model
    Handle<spr::Model> modelHandle = rm.getHandle<spr::Model>(spr::data::default_model);
    if (!modelHandle.isValid()){
        SprLog::warn("[GfxAssetLoader] [loadBuiltinAssets] invalid model");
        return;
    }
    spr::Model* model = rm.getData<spr::Model>(modelHandle);
    
    Handle<spr::Mesh> meshHandle = rm.getHandle<spr::Mesh>(model->meshIds[0]);
    if (!meshHandle.isValid()){
        SprLog::warn("[GfxAssetLoader] [loadBuiltinAssets] invalid mesh");
        return;
    }
    spr::Mesh* mesh = rm.getData<spr::Mesh>(meshHandle);
    
    MeshInfo info;
    loadVertexData(rm, mesh, info);
    loadMaterial(rm, mesh, info);
    meshes[0] = info;

    // create built-in quad mesh
    std::vector<VertexPosition> quadPos = {
        {{-1.0f,  1.0f, 0.0f, 1.0f}},
        {{-1.0f, -1.0f, 0.0f, 1.0f}},
        {{ 1.0f, -1.0f, 0.0f, 1.0f}},
        {{ 1.0f,  1.0f, 0.0f, 1.0f}}
    };
    std::vector<VertexAttributes> quadAttr = {
        {{0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
    };
    std::vector<uint32> quadIndices = {
        0, 1, 3, 3, 1, 2
    };
    
    auto quadIdxAlloc = m_vertexIndices.allocateAndInsert<uint32>({
        .data = quadIndices.data(),
        .size = (uint32)(quadIndices.size())
    });
    
    auto quadPosAlloc = m_vertexPositions.allocateAndInsert<VertexPosition>({
        .data = quadPos.data(),
        .size = (uint32)(quadPos.size())
    });
    
    auto quadAttrAlloc = m_vertexAttributes.allocateAndInsert<VertexAttributes>({
        .data = quadAttr.data(),
        .size = (uint32)(quadAttr.size())
    });
    
    MeshInfo quadInfo;
    quadInfo.firstIndex = quadIdxAlloc.offset;
    quadInfo.indexCount = quadIdxAlloc.size;
    m_counts.indexCount += quadInfo.indexCount;
    quadInfo.vertexOffset = quadPosAlloc.offset;
    m_counts.vertexCount += quadPosAlloc.size;
    m_counts.bytes += quadIdxAlloc.byteSize + quadPosAlloc.byteSize + quadAttrAlloc.byteSize;
    meshes[1] = quadInfo;

    // create built-in cube mesh
    std::vector<VertexPosition> cubePos = {
        // Z-
        {{ 1.0f,  1.0f,  1.0f, 1.0f}},
        {{ 1.0f,  1.0f, -1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f, 1.0f}},
        // Z+
        {{-1.0f, -1.0f,  1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f, 1.0f}},
        {{ 1.0f, -1.0f,  1.0f, 1.0f}},
        // X+
        {{ 1.0f,  1.0f,  1.0f, 1.0f}},
        {{ 1.0f, -1.0f,  1.0f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f, 1.0f}},
        {{ 1.0f,  1.0f, -1.0f, 1.0f}},
        // X-
        {{-1.0f, -1.0f,  1.0f, 1.0f}},
        {{-1.0f,  1.0f,  1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f, 1.0f}},
        // Y+
        {{-1.0f,  1.0f,  1.0f, 1.0f}},
        {{-1.0f, -1.0f,  1.0f, 1.0f}},
        {{ 1.0f, -1.0f,  1.0f, 1.0f}},
        {{ 1.0f,  1.0f,  1.0f, 1.0f}},
        // Y-
        {{ 1.0f,  1.0f, -1.0f, 1.0f}},
        {{ 1.0f, -1.0f, -1.0f, 1.0f}},
        {{-1.0f, -1.0f, -1.0f, 1.0f}},
        {{-1.0f,  1.0f, -1.0f, 1.0f}},
        
    };
    std::vector<VertexAttributes> cubeAttr = {
        // Z-
        {{ 0.0f,  0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 0.0f,  0.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 0.0f,  0.0f, -1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ 0.0f,  0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        // Z+
        {{ 0.0f,  0.0f,  1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 0.0f,  0.0f,  1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 0.0f,  0.0f,  1.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ 0.0f,  0.0f,  1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        // X+
        {{ 1.0f,  0.0f,  0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 1.0f,  0.0f,  0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 1.0f,  0.0f,  0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ 1.0f,  0.0f,  0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        // X-
        {{-1.0f,  0.0f,  0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{-1.0f,  0.0f,  0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{-1.0f,  0.0f,  0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{-1.0f,  0.0f,  0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        // Y+
        {{ 0.0f,  1.0f,  0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 0.0f,  1.0f,  0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 0.0f,  1.0f,  0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ 0.0f,  1.0f,  0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        // Y-
        {{ 0.0f, -1.0f,  0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 0.0f, -1.0f,  0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 0.0f}},
        {{ 0.0f, -1.0f,  0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
        {{ 0.0f, -1.0f,  0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
    };
    std::vector<uint32> cubeIndices = {
        0,  1,  3,  3,  1,  2,
        4,  5,  7,  7,  5,  6,
        8,  9,  11, 11, 9,  10, 
        12, 13, 15, 15, 13, 14,
        16, 17, 19, 19, 17, 18,
        20, 21, 23, 23, 21, 22
    };

    auto cubeIdxAlloc = m_vertexIndices.allocateAndInsert<uint32>({
        .data = cubeIndices.data(),
        .size = (uint32)(cubeIndices.size())
    });

    auto cubePosAlloc = m_vertexPositions.allocateAndInsert<VertexPosition>({
        .data = cubePos.data(),
        .size = (uint32)(cubePos.size())
    });
    
    auto cubeAttrAlloc = m_vertexAttributes.allocateAndInsert<VertexAttributes>({
        .data = cubeAttr.data(),
        .size = (uint32)(cubeAttr.size())
    });
    
    MeshInfo cubeInfo;
    cubeInfo.firstIndex = cubeIdxAlloc.offset;
    cubeInfo.indexCount = cubeIdxAlloc.size;
    m_counts.indexCount += cubeInfo.indexCount;
    cubeInfo.vertexOffset = cubePosAlloc.offset;
    m_counts.vertexCount += cubePosAlloc.size;
    m_counts.bytes += cubeIdxAlloc.byteSize + cubePosAlloc.byteSize + cubeAttrAlloc.byteSize;
    meshes[2] = cubeInfo;
}

void GfxAssetLoader::unloadBuffers(SprResourceManager& rm){
    for (Handle<spr::Buffer> handle : m_bufferHandles){
        rm.deleteData(handle);
    }
    m_bufferHandles.clear();
    m_storedBuffersBytes = 0;
}

void GfxAssetLoader::clearCubemaps(){
    for (TextureInfo& textureInfo : m_cubemaps){
        m_rm->remove(textureInfo.data.handle());
    }
    m_cubemaps.clear();
}

void GfxAssetLoader::clearTextures(){
    for (TextureInfo& textureInfo : m_textures){
        m_rm->remove(textureInfo.data.handle());
    }
    m_textures.clear();
}

void GfxAssetLoader::clearMaterials(){
    m_materials.destroy();
}

void GfxAssetLoader::clearVertexIndices(){
    m_vertexIndices.destroy();
}

void GfxAssetLoader::clearVertexAttributes(){
    m_vertexAttributes.destroy();
}

void GfxAssetLoader::clearVertexPositions(){
    m_vertexPositions.destroy();
}


void GfxAssetLoader::clear(){
    if (m_cleared)
        return;
    m_vertexPositions.destroy();
    m_vertexAttributes.destroy();
    m_vertexIndices.destroy();
    m_materials.destroy();
    for (TextureInfo& textureInfo : m_textures){
        m_rm->remove(textureInfo.data.handle());
    }
    m_textures.clear();
    for (TextureInfo& textureInfo : m_cubemaps){
        m_rm->remove(textureInfo.data.handle());
    }
    m_cubemaps.clear();
    m_cleared = true;
}

PrimitiveCounts GfxAssetLoader::getPrimitiveCounts(){
    return m_counts;
}

Handle<Buffer> GfxAssetLoader::getVertexPositionData(){
    return m_vertexPositions.handle();
}

Handle<Buffer> GfxAssetLoader::getVertexAttributeData(){
    return m_vertexAttributes.handle();
}

Handle<Buffer> GfxAssetLoader::getVertexIndicesData(){
    return m_vertexIndices.handle();
}

Handle<Buffer> GfxAssetLoader::getMaterialData(){
    return m_materials.handle();
}

std::vector<TextureInfo>& GfxAssetLoader::getTextureData(){
    return m_textures;
}

std::vector<TextureInfo>& GfxAssetLoader::getCubemapData(){
    return m_cubemaps;
}

}