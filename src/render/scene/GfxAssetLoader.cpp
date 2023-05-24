#include "GfxAssetLoader.h"
#include "Material.h"
#include "Mesh.h"
#include "../../resource/SprResourceManager.h"
#include "../../debug/SprLog.h"

namespace spr::gfx {

GfxAssetLoader::GfxAssetLoader() : 
    m_vertexPositions(10000000),
    m_vertexAttributes(10000000),
    m_vertexIndices(10000000),
    m_materials(2048){
}

GfxAssetLoader::~GfxAssetLoader(){
    if (!m_cleared)
        clear();
}

MeshInfoMap GfxAssetLoader::loadAssets(SprResourceManager& rm){
    std::vector<uint32>& modelIds = rm.getModelIds();
    std::vector<uint32>& textureIds = rm.getTextureIds();

    MeshInfoMap map;

    // built-in assets
    loadBuiltinAssets(rm, map);

    // iterate over models and load subresources
    for (uint32 modelId : modelIds){
        Handle<spr::Model> modelHandle = rm.getHandle<spr::Model>(modelId);
        if (!modelHandle.isValid()){
            SprLog::warn("[GfxAssetLoader] [loadAssets] invalid model");
        }
        spr::Model* model = rm.getData<spr::Model>(modelHandle);
        // model's meshes
        for (uint32 meshId : model->meshIds){
            Handle<spr::Mesh> meshHandle = rm.getHandle<spr::Mesh>(meshId);
            if (!meshHandle.isValid()){
                SprLog::warn("[GfxAssetLoader] [loadAssets] invalid mesh");
            }
            spr::Mesh* mesh = rm.getData<spr::Mesh>(meshHandle);
            MeshInfo meshInfo;
            loadVertexData(rm, mesh, meshInfo);
            loadMaterial(rm, mesh, meshInfo);

            map[meshId] = meshInfo;
        }
    }

    // iterate over non-subresource textures
    // for (uint32 texId : textureIds){
    //     uint32 textureIndex = loadTexture(rm, texId, true);
    // }

    return map;
}

void GfxAssetLoader::loadVertexData(SprResourceManager& rm, Mesh* mesh, MeshInfo& info){
    // indices
    if (mesh->indexBufferId > 0){
        Handle<spr::Buffer> indicesHandle = rm.getHandle<spr::Buffer>(mesh->indexBufferId);
        if (!indicesHandle.isValid()){
            SprLog::warn("[GfxAssetLoader] [loadVertexData] invalid indices");
            return;
        }
        spr::Buffer* indicesBuffer = rm.getData<spr::Buffer>(indicesHandle);
        uint32 typedSize = (indicesBuffer->byteLength)/sizeof(uint16);
        info.indexCount = typedSize;
        info.firstIndex = m_vertexIndices.insert((uint16*)(indicesBuffer->data.data()), typedSize); 
        m_counts.indexCount += info.indexCount;
    }

    // position
    uint32 vertexCount = 0;
    if (mesh->positionBufferId > 0){
        Handle<spr::Buffer> positionHandle = rm.getHandle<spr::Buffer>(mesh->positionBufferId);
        if (!positionHandle.isValid()){
            SprLog::warn("[GfxAssetLoader] [loadVertexData] invalid positions");
            return;
        }
        spr::Buffer* positionBuffer = rm.getData<spr::Buffer>(positionHandle);
        uint32 typedSize = (positionBuffer->byteLength)/sizeof(VertexPosition);
        info.vertexOffset = m_vertexPositions.insert((VertexPosition*)(positionBuffer->data.data()), typedSize);
        m_counts.vertexCount += typedSize;
        vertexCount = typedSize;
    }

    // attributes
    if (mesh->attributesBufferId > 0){
        Handle<spr::Buffer> attributesHandle = rm.getHandle<spr::Buffer>(mesh->attributesBufferId);
        if (!attributesHandle.isValid()){
            SprLog::warn("[GfxAssetLoader] [loadVertexData] invalid attributes");
            return;
        }
        spr::Buffer* attributesBuffer = rm.getData<spr::Buffer>(attributesHandle);
        uint32 typedSize = (attributesBuffer->byteLength)/sizeof(VertexAttributes);
        uint32 offset = m_vertexAttributes.insert((VertexAttributes*)(attributesBuffer->data.data()), typedSize);
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

    info.materialIndex = m_materials.insert(materialData);
    m_counts.materialCount++;

}

uint32 GfxAssetLoader::loadTexture(SprResourceManager& rm, uint32 textureId, bool srgb){
    Handle<spr::Texture> handle = rm.getHandle<spr::Texture>(textureId);
    if (!handle.isValid()){
        SprLog::warn("[GfxAssetLoader] [loadTexture] invalid texture");
        return 0;
    }

    spr::Texture* texture = rm.getData<spr::Texture>(handle);
    Handle<spr::Buffer> texBufferHandle = rm.getHandle<spr::Buffer>(texture->bufferId);
    if (!texBufferHandle.isValid()){
        SprLog::warn("[GfxAssetLoader] [loadTexture] invalid buffer");
        return 0;
    }

    if (m_textureIds.count(texture->bufferId) > 0){
        return m_textureIds[texture->bufferId];
    }

    spr::Buffer* texBuffer = rm.getData<spr::Buffer>(texBufferHandle);
    TempBuffer<uint8> textureBuffer(texBuffer->data.size());
    textureBuffer.insert(texBuffer->data.data(), texBuffer->data.size());

    uint32 texIndex = m_textures.size();
    m_textures.push_back({
        .data = textureBuffer,
        .height = texture->height,
        .width = texture->width,
        .components = texture->components,
        .srgb = srgb});

    m_counts.textureCount++;
    m_textureIds[texture->bufferId] = texIndex;

    return texIndex;
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
        {{0.0f, 0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f, 1.0f}},
        {{0.0f, 0.0f, 1.0f, 0.0f},{1.0f, 1.0f, 1.0f, 1.0f}}
    };
    std::vector<uint16> quadIndices = {
        0,1,3,3,1,2
    };

    MeshInfo quadInfo;
    quadInfo.indexCount = quadIndices.size();
    quadInfo.firstIndex = m_vertexIndices.insert((uint16*)(quadIndices.data()), quadIndices.size()); 
    quadInfo.vertexOffset = m_vertexPositions.insert((VertexPosition*)(quadPos.data()), quadPos.size());
    m_vertexAttributes.insert((VertexAttributes*)(quadAttr.data()), quadAttr.size());
    m_counts.indexCount += quadInfo.indexCount;
    m_counts.vertexCount += quadPos.size();
    meshes[1] = quadInfo;
}

void GfxAssetLoader::clear(){
    m_vertexPositions.reset();
    m_vertexAttributes.reset();
    m_vertexIndices.reset();
    m_materials.reset();
    m_textures.clear();
    m_cleared = true;
}

PrimitiveCounts GfxAssetLoader::getPrimitiveCounts(){
    return m_counts;
}

TempBuffer<VertexPosition>& GfxAssetLoader::getVertexPositionData(){
    return m_vertexPositions;
}

TempBuffer<VertexAttributes>& GfxAssetLoader::getVertexAttributeData(){
    return m_vertexAttributes;
}

TempBuffer<uint16>& GfxAssetLoader::getVertexIndicesData(){
    return m_vertexIndices;
}

TempBuffer<MaterialData>& GfxAssetLoader::getMaterialData(){
    return m_materials;
}

std::vector<TextureInfo>& GfxAssetLoader::getTextureData(){
    return m_textures;
}

}