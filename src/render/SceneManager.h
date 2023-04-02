#pragma once

#include "glm/fwd.hpp"
#include "spruce_core.h"
#include "scene/BatchManager.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include <vector>
#include "vulkan/VulkanRenderer.h"
#include "scene/GfxAssetLoader.h"


namespace spr::gfx {



class SceneManager {
public:
    SceneManager();
    SceneManager(VulkanResourceManager& rm);
    ~SceneManager();

    void insertMesh(uint32 frame, uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose);
    void insertLight(uint32 frame, Light light);
    void updateCamera(uint32 frame, Camera camera);
    void reset(uint32 frame);

    void initializeAssets(SprResourceManager& rm);

    Handle<DescriptorSet> getGlobalDescriptorSet();
    Handle<DescriptorSet> getPerFrameDescriptorSet(uint32 frame);

    void uploadGlobalResources();
    void uploadPerFrameResources(uint32 frame);

    BatchManager& getBatchManager(uint32 frame);

    void destroy();


private:
    VulkanResourceManager* m_rm;
    GfxAssetLoader m_assetLoader;
    BatchManager m_batchManagers[MAX_FRAME_COUNT];
    MeshInfoMap m_meshInfo;
    bool m_destroyed = false;

    void initBuffers(PrimitiveCounts counts);

private: // owning
    // per frame resource handles
    Handle<Buffer> m_lightsBuffer;
    Handle<Buffer> m_transformBuffer;
    Handle<Buffer> m_drawDataBuffer;
    Handle<Buffer> m_cameraBuffer;
    Handle<Buffer> m_sceneBuffer;

    // global resource handles
    Handle<Buffer> m_positionsBuffer;  
    Handle<Buffer> m_attributesBuffer;
    Handle<Buffer> m_indexBuffer;
    Handle<Buffer> m_materialsBuffer;

    // per-frame resource tempbuffers
    TempBuffer<DrawData> m_drawData[MAX_FRAME_COUNT];
    TempBuffer<Camera> m_cameras[MAX_FRAME_COUNT];
    TempBuffer<Scene> m_sceneData[MAX_FRAME_COUNT];
    TempBuffer<Light> m_lights[MAX_FRAME_COUNT];
    TempBuffer<Transform> m_transforms[MAX_FRAME_COUNT];
};
}