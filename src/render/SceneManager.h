#pragma once

#include "glm/fwd.hpp"
#include "spruce_core.h"
#include "scene/BatchManager.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include <vector>
#include "scene/SceneData.h"
#include "scene/Mesh.h"
#include "../../external/flat_hash_map/flat_hash_map.hpp"
#include "../resource/ResourceManager.h"
#include "vulkan/VulkanRenderer.h"


namespace spr::gfx {

typedef ska::flat_hash_map<uint32, MeshInfo> mmap;

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void insertMesh(uint32 frame, uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose);
    void insertLight(uint32 frame, Light light);
    void updateCamera(uint32 frame, Camera camera);
    void reset(uint32 frame);

    void initializeAssets(ResourceManager& rm, VulkanRenderer& renderer);

    Handle<DescriptorSet> getGlobalDescriptorSet();
    Handle<DescriptorSet> getPerFrameDescriptorSet(uint32 frame);

    void uploadGlobalResources();
    void uploadPerFrameResources(uint32 frame);

    BatchManager& getBatchManager(uint32 frame);

private:
    BatchManager m_batchManagers[MAX_FRAME_COUNT];
    mmap m_meshInfo;

    // per frame resource handles
    Handle<Buffer> m_lightsBuffer;
    Handle<Buffer> m_transformBuffer;
    Handle<Buffer> m_drawDataBuffer;
    Handle<Buffer> m_cameraBuffer;
    Handle<Buffer> m_sceneBuffer;

    // global resource handles
    Handle<Buffer> m_positionsBuffer;
    Handle<Buffer> m_attributesBuffer;
    Handle<MaterialData> m_materialsBuffer;
    Handle<Buffer> m_textureDescBuffer;

    // per-frame resource tempbuffers
    TempBuffer<DrawData> m_drawData[MAX_FRAME_COUNT];
    TempBuffer<Camera> m_cameras[MAX_FRAME_COUNT];
    TempBuffer<Scene> m_sceneData[MAX_FRAME_COUNT];
    TempBuffer<Light> m_lights[MAX_FRAME_COUNT];
    TempBuffer<Transform> m_transforms[MAX_FRAME_COUNT];
};
}