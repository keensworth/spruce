#include "GfxAssetLoader.h"
#include "Material.h"

namespace spr::gfx {

GfxAssetLoader::GfxAssetLoader(){

}

GfxAssetLoader::~GfxAssetLoader(){

}

MeshInfoMap GfxAssetLoader::loadAssets(SprResourceManager& rm){
    std::vector<uint32>& modelIds = rm.getModelIds();
    std::vector<uint32>& textureIds = rm.getTextureIds();

    // load default textures
    loadTexture(rm, spr::data::default_color);
    loadTexture(rm, spr::data::default_input_black);
    loadTexture(rm, spr::data::default_input_white);

    // iterate over models and load subresources
    MeshInfoMap map;
    for (uint32 modelId : modelIds){
        Handle<spr::Model> modelHandle = rm.getHandle<spr::Model>(modelId);
        spr::Model* model = rm.getData<spr::Model>(modelHandle);

        // model's meshes
        for (uint32 meshId : model->meshIds){
            Handle<spr::Mesh> meshHandle = rm.getHandle<spr::Mesh>(meshId);
            spr::Mesh* mesh = rm.getData<spr::Mesh>(meshHandle);

            MeshInfo meshInfo;
            loadVertexData(rm, mesh, meshInfo);
            loadMaterial(rm, mesh, meshInfo);

            map[meshId] = meshInfo;
        }
    }

    // iterate over non-subresource textures
    for (uint32 texId : textureIds){
        uint32 textureIndex = loadTexture(rm, texId);
        // TODO: store tmap
    }

    return map;
}

void GfxAssetLoader::loadVertexData(SprResourceManager& rm, Mesh* mesh, MeshInfo& info){
    // process the mesh's vertex data
    if (mesh->indexBufferId > 0){
        Handle<spr::Buffer> indicesHandle = rm.getHandle<spr::Buffer>(mesh->indexBufferId);
        spr::Buffer* indicesBuffer = rm.getData<spr::Buffer>(indicesHandle);
        uint32 typedSize = (indicesBuffer->byteLength)/sizeof(uint32);
        info.indexCount = typedSize;
        info.firstIndex = m_vertexIndices.insert((uint32*)(indicesBuffer->data.data()), typedSize); 
        m_counts.indexCount += info.indexCount;
    }
    if (mesh->positionBufferId > 0){
        Handle<spr::Buffer> positionHandle = rm.getHandle<spr::Buffer>(mesh->positionBufferId);
        spr::Buffer* positionBuffer = rm.getData<spr::Buffer>(positionHandle);
        uint32 typedSize = (positionBuffer->byteLength)/sizeof(VertexPosition);
        info.vertexOffset = m_vertexPositions.insert((VertexPosition*)(positionBuffer->data.data()), typedSize);
        m_counts.vertexCount += typedSize;
    }
    if (mesh->attributesBufferId > 0){
        Handle<spr::Buffer> attributesHandle = rm.getHandle<spr::Buffer>(mesh->positionBufferId);
        spr::Buffer* attributesBuffer = rm.getData<spr::Buffer>(attributesHandle);
        uint32 typedSize = (attributesBuffer->byteLength)/sizeof(VertexAttributes);
        m_vertexAttributes.insert((VertexAttributes*)(attributesBuffer->data.data()), typedSize);
    }
}

void GfxAssetLoader::loadMaterial(SprResourceManager& rm, Mesh* mesh, MeshInfo& info){
    // process the mesh's material
    Handle<spr::Material> materialHandle = rm.getHandle<spr::Material>((mesh->materialId));
    spr::Material* material = rm.getData(materialHandle);

    MaterialData materialData;
    if (material->baseColorTexId > 0){
        materialData.flags |= MTL_BASE_COLOR;
        materialData.baseColorTexIdx = loadTexture(rm, material->baseColorTexId);
        materialData.baseColorFactor = material->baseColorFactor;
    }
    if (material->metalRoughTexId > 0){
        materialData.flags |= MTL_METALLIC_ROUGHNESS;
        materialData.metalRoughTexIdx = loadTexture(rm, material->metalRoughTexId);
        materialData.metallicFactor = material->metalFactor;
    }
    if (material->normalTexId > 0){
        materialData.flags |= MTL_NORMAL;
        materialData.normalTexIdx = loadTexture(rm, material->normalTexId);
        materialData.normalScale = material->normalScale;
    }
    if (material->occlusionTexId > 0){
        materialData.flags |= MTL_OCCLUSION;
        materialData.occlusionTexIdx = loadTexture(rm, material->occlusionTexId);
        materialData.occlusionStrength = material->occlusionStrength;
    }
    if (material->emissiveTexId > 0){
        materialData.flags |= MTL_EMISSIVE;
        materialData.emissiveTexIdx = loadTexture(rm, material->emissiveTexId);
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

uint32 GfxAssetLoader::loadTexture(SprResourceManager& rm, uint32 textureId){
    Handle<spr::Texture> handle = rm.getHandle<spr::Texture>(textureId);
    spr::Texture* texture = rm.getData<spr::Texture>(handle);

    Handle<spr::Buffer> texBufferHandle = rm.getHandle<spr::Buffer>(texture->bufferId);
    spr::Buffer* texBuffer = rm.getData<spr::Buffer>(texBufferHandle);
    
    TempBuffer<uint8> textureBuffer;
    textureBuffer.insert(texBuffer->data.data(), texBuffer->byteLength);

    uint32 texIndex = m_textures.size();
    m_textures.push_back(textureBuffer);

    m_counts.textureCount++;
    return texIndex;
}

void GfxAssetLoader::clear(){

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

TempBuffer<uint32>& GfxAssetLoader::getVertexIndicesData(){
    return m_vertexIndices;
}

TempBuffer<MaterialData>& GfxAssetLoader::getMaterialData(){
    return m_materials;
}

std::vector<TempBuffer<uint8>>& GfxAssetLoader::getTextureDta(){
    return m_textures;
}

}