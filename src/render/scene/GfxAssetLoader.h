#pragma once

#include "core/memory/TempBuffer.h"
#include "core/util/Span.h"
#include "external/flat_hash_map/flat_hash_map.hpp"
#include "render/scene/Mesh.h"
#include "vulkan/TextureTranscoder.h"
#include "vulkan/resource/OffsetBuffer.h"

namespace spr {
    class SprResourceManager;
    struct Mesh;
    struct Buffer;
    template <class T = Buffer>
    class Handle;
}

namespace spr::gfx {

struct MeshInfo;
struct VertexPosition;
struct VertexAttributes;
struct MaterialData;

typedef ska::flat_hash_map<uint32, MeshInfo> MeshInfoMap;

struct TextureInfo {
    OffsetBuffer data;
    uint32 height;
    uint32 width;
    uint32 components;
    uint32 format;
    uint32 mipCount;
    uint32 layerCount;
    bool srgb;
};

struct PrimitiveCounts {
    uint32 vertexCount   = 0;
    uint32 indexCount    = 0;
    uint32 materialCount = 0;
    uint32 textureCount  = 0;
    uint32 cubemapCount  = 0;
    uint64 bytes         = 0;
};

class GfxAssetLoader {
public:
    GfxAssetLoader();
    ~GfxAssetLoader();

    MeshInfoMap loadAssets(SprResourceManager& rm, VulkanResourceManager* vrm, VulkanDevice* device);
    void unloadBuffers(SprResourceManager& rm);
    void clear();

    PrimitiveCounts getPrimitiveCounts();

    Handle<Buffer> getVertexPositionData();
    Handle<Buffer> getVertexAttributeData();
    Handle<Buffer> getVertexIndicesData();
    Handle<Buffer> getMaterialData();
    std::vector<TextureInfo>& getTextureData();
    std::vector<TextureInfo>& getCubemapData();

    void clearCubemaps();
    void clearTextures();
    void clearMaterials();
    void clearVertexIndices();
    void clearVertexAttributes();
    void clearVertexPositions();

private:
    TextureTranscoder m_transcoder;
    PrimitiveCounts m_counts;
    VulkanResourceManager* m_rm;

    // copy of asset data for upload to GPU
    OffsetBuffer m_vertexPositions;
    OffsetBuffer m_vertexAttributes;
    OffsetBuffer m_vertexIndices;
    OffsetBuffer m_materials;
    std::vector<TextureInfo> m_textures;
    std::vector<TextureInfo> m_cubemaps;
    bool m_cleared = false;

    std::vector<Handle<spr::Buffer>> m_bufferHandles;
    uint32 m_storedBuffersBytes = 0;
    uint32 MAX_STORED_BUFFER_BYTES = 1 << 29;

    void loadVertexData(SprResourceManager& rm, Mesh* mesh, MeshInfo& info);
    void loadMaterial(SprResourceManager& rm, Mesh* mesh, MeshInfo& info);
    uint32 loadTexture(SprResourceManager& rm, uint32 texId, bool srgb);
    void loadBuiltinAssets(SprResourceManager& rm, MeshInfoMap& meshes);

    ska::flat_hash_map<uint32, uint32> m_textureIds;
    ska::flat_hash_map<uint32, uint32> m_cubemapIds;
    ska::flat_hash_map<uint32, uint32> m_indexBufferIds;
    ska::flat_hash_map<uint32, uint32> m_positionBufferIds;
    ska::flat_hash_map<uint32, uint32> m_attributeBufferIds;
};
}