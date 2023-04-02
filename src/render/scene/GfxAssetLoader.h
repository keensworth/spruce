#pragma once

#include "../../resource/SprResourceManager.h"
#include "../vulkan/resource/VulkanResourceManager.h"
#include "SceneData.h"
#include "Mesh.h"
#include "Material.h"
#include "../../core/memory/TempBuffer.h"
#include "../../external/flat_hash_map/flat_hash_map.hpp"

namespace spr::gfx {

typedef ska::flat_hash_map<uint32, MeshInfo> MeshInfoMap;

typedef struct PrimitiveCounts {
    uint32 vertexCount   = 0;
    uint32 indexCount    = 0;
    uint32 materialCount = 0;
    uint32 textureCount  = 0;
} PrimitiveCounts;

class GfxAssetLoader {
public:
    GfxAssetLoader();
    ~GfxAssetLoader();

    MeshInfoMap loadAssets(SprResourceManager& rm);
    void clear();

    PrimitiveCounts getPrimitiveCounts();

    TempBuffer<VertexPosition>& getVertexPositionData();
    TempBuffer<VertexAttributes>& getVertexAttributeData();
    TempBuffer<uint32>& getVertexIndicesData();
    TempBuffer<MaterialData>& getMaterialData();
    std::vector<TempBuffer<uint8>>& getTextureDta();

private:
    SprResourceManager* m_rm;
    PrimitiveCounts m_counts;

    TempBuffer<VertexPosition> m_vertexPositions;
    TempBuffer<VertexAttributes> m_vertexAttributes;
    TempBuffer<uint32> m_vertexIndices;
    TempBuffer<MaterialData> m_materials;
    std::vector<TempBuffer<uint8>> m_textures;

    void loadVertexData(SprResourceManager& rm, Mesh* mesh, MeshInfo& info);
    void loadMaterial(SprResourceManager& rm, Mesh* mesh, MeshInfo& info);
    uint32 loadTexture(SprResourceManager& rm, uint32 texId);
};
}