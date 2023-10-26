#pragma once

#include "scene/BatchManager.h"
#include <vector>
#include "scene/GfxAssetLoader.h"
#include "../core/memory/Handle.h"
#include "../core/util/Span.h"
#include "scene/SceneData.h"
#include "vulkan/gfx_vulkan_core.h"


namespace spr {
    class SprResourceManager;
}

namespace spr::gfx {

class VulkanDevice;
class VulkanResourceManager;
class UploadHandler;
struct DescriptorSet;
struct DescriptorSetLayout;
struct Buffer;
struct Texture;

typedef struct TransformUpdate {
    uint32 index = 0;
    uint32 budget = MAX_FRAME_COUNT;
} TransformUpdate;

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void init(VulkanResourceManager& rm);

    void insertDraws(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags, uint32 transformIndex, bool sharedMaterial);

    void insertMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags, const Transform& transform);
    void insertMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, uint32 materialFlags, const Transform& transform);

    void insertMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags);
    void insertMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, uint32 materialFlags);

    void updateMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags, const Transform& transform);
    void updateMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, uint32 materialFlags , const Transform& transform);

    void insertLights(uint32 frame, Span<const Light> lights);
    
    void updateCamera(uint32 frame, glm::vec2 screenDim, const Camera& camera);
    
    void reset(uint32 frame);

    void initializeAssets(SprResourceManager& rm, VulkanDevice* device);

    Handle<DescriptorSet> getGlobalDescriptorSet();
    Handle<DescriptorSetLayout> getGlobalDescriptorSetLayout();
    Handle<DescriptorSet> getPerFrameDescriptorSet();
    Handle<DescriptorSetLayout> getPerFrameDescriptorSetLayout();
    Handle<Buffer> getIndexBuffer();

    Scene& getScene(uint32 frame);
    Camera& getCamera(uint32 frame);
    Light& getSunLight(uint32 frame);

    void uploadGlobalResources(UploadHandler& uploadHandler);
    void uploadPerFrameResources(UploadHandler& uploadHandler, uint32 frame);

    BatchManager& getBatchManager(uint32 frame);

    void destroy();


private:
    VulkanResourceManager* m_rm;
    GfxAssetLoader m_assetLoader;
    BatchManager m_batchManagers[MAX_FRAME_COUNT];
    MeshInfoMap m_meshInfo;
    bool m_destroyed = false;

    ska::flat_hash_map<uint32, uint32> m_idTransformIndexMap;
    std::vector<TransformUpdate> m_transformUpdates;
    std::deque<uint32> m_updatesFreelist;

    void initBuffers(PrimitiveCounts counts, VulkanDevice* device);
    void initTextures(PrimitiveCounts counts, VulkanDevice* device);
    void initDescriptorSets(VulkanDevice* device);

    void queueTransformUpdate(uint32 index);

private: // owning
    // per frame resource handles
    Handle<Buffer> m_lightsBuffer;
    Handle<Buffer> m_transformBuffer;
    Handle<Buffer> m_drawDataBuffer;
    Handle<Buffer> m_cameraBuffer;
    Handle<Buffer> m_sceneBuffer;
    Handle<DescriptorSetLayout> m_frameDescriptorSetLayout;
    Handle<DescriptorSet> m_frameDescriptorSet;

    // global resource handles
    Handle<Buffer> m_positionsBuffer;  
    Handle<Buffer> m_attributesBuffer;
    Handle<Buffer> m_indexBuffer;
    Handle<Buffer> m_materialsBuffer;
    std::vector<Handle<Texture>> m_textures;
    std::vector<Handle<Texture>> m_cubemaps;
    Handle<DescriptorSetLayout> m_globalDescriptorSetLayout;
    Handle<DescriptorSet> m_globalDescriptorSet;

    // per-frame resource tempbuffers
    TempBuffer<DrawData> m_drawData[MAX_FRAME_COUNT];
    TempBuffer<Camera> m_cameras[MAX_FRAME_COUNT];
    TempBuffer<Scene> m_sceneData[MAX_FRAME_COUNT];
    TempBuffer<Light> m_lights[MAX_FRAME_COUNT];
    TempBuffer<Transform> m_transforms[MAX_FRAME_COUNT];

};
}