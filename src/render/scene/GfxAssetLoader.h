#pragma once

#include "../../core/memory/TempBuffer.h"
#include "../../external/flat_hash_map/flat_hash_map.hpp"
#include "Mesh.h"
#include "vulkan/TextureTranscoder.h"

namespace spr {
    class SprResourceManager;
    struct Mesh;
}

namespace spr::gfx {

struct MeshInfo;
struct VertexPosition;
struct VertexAttributes;
struct MaterialData;

typedef ska::flat_hash_map<uint32, MeshInfo> MeshInfoMap;

typedef struct TextureInfo {
    TempBuffer<uint8> data;
    uint32 height;
    uint32 width;
    uint32 components;
    uint32 format;
    uint32 mipCount;
    bool srgb;
} TextureInfo;

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

    MeshInfoMap loadAssets(SprResourceManager& rm, VulkanDevice* device);
    void clear();

    PrimitiveCounts getPrimitiveCounts();

    TempBuffer<VertexPosition>& getVertexPositionData();
    TempBuffer<VertexAttributes>& getVertexAttributeData();
    TempBuffer<uint16>& getVertexIndicesData();
    TempBuffer<MaterialData>& getMaterialData();
    std::vector<TextureInfo>& getTextureData();

private:
    SprResourceManager* m_rm;
    TextureTranscoder m_transcoder;
    PrimitiveCounts m_counts;

    TempBuffer<VertexPosition> m_vertexPositions;
    TempBuffer<VertexAttributes> m_vertexAttributes;
    TempBuffer<uint16> m_vertexIndices;
    TempBuffer<MaterialData> m_materials;
    std::vector<TextureInfo> m_textures;
    bool m_cleared = false;

    void loadVertexData(SprResourceManager& rm, Mesh* mesh, MeshInfo& info);
    void loadMaterial(SprResourceManager& rm, Mesh* mesh, MeshInfo& info);
    uint32 loadTexture(SprResourceManager& rm, uint32 texId, bool srgb);
    void loadBuiltinAssets(SprResourceManager& rm, MeshInfoMap& meshes);

    ska::flat_hash_map<uint32, uint32> m_textureIds;
};
}