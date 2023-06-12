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

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void init(VulkanResourceManager& rm);

    void insertLights(uint32 frame, Span<const Light> lights);
    void updateCamera(uint32 frame, glm::vec2 screenDim, const Camera& camera);

    void insertMeshes(uint32 frame, Span<uint32> meshIds, Span<uint32> materialsFlags, Span<const Transform> transforms);
    void insertMeshes(uint32 frame, Span<uint32> meshIds, Span<uint32> materialsFlags,      const Transform& transform );
    void insertMeshes(uint32 frame, Span<uint32> meshIds,      uint32 materialFlags  , Span<const Transform> transforms);
    void insertMeshes(uint32 frame, Span<uint32> meshIds,      uint32 materialFlags  ,      const Transform& transform );
    
    void reset(uint32 frame);

    void initializeAssets(SprResourceManager& rm, VulkanDevice* device);

    Handle<DescriptorSet> getGlobalDescriptorSet();
    Handle<DescriptorSetLayout> getGlobalDescriptorSetLayout();
    Handle<DescriptorSet> getPerFrameDescriptorSet(uint32 frame);
    Handle<DescriptorSet>* getPerFrameDescriptorSets();
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

    void initBuffers(PrimitiveCounts counts, VulkanDevice* device);
    void initTextures(uint32 textureCount);
    void initDescriptorSets(VulkanDevice* device);

private: // owning
    // per frame resource handles
    Handle<Buffer> m_lightsBuffer;
    Handle<Buffer> m_transformBuffer;
    Handle<Buffer> m_drawDataBuffer;
    Handle<Buffer> m_cameraBuffer;
    Handle<Buffer> m_sceneBuffer;
    Handle<DescriptorSetLayout> m_frameDescriptorSetLayout;
    Handle<DescriptorSet> m_frameDescriptorSets[MAX_FRAME_COUNT];

    // global resource handles
    Handle<Buffer> m_positionsBuffer;  
    Handle<Buffer> m_attributesBuffer;
    Handle<Buffer> m_indexBuffer;
    Handle<Buffer> m_materialsBuffer;
    std::vector<Handle<Texture>> m_textures;
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