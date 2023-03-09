#pragma once

#include "glm/fwd.hpp"
#include "spruce_core.h"
#include "BatchManager.h"
#include "../vulkan/resource/VulkanResourceManager.h"
#include <vector>
#include "SceneData.h"
#include "../../core/memory/TempBuffer.h"
#include "../../../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::gfx {

typedef ska::flat_hash_map<uint32, MeshData> mmap;

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void insertMesh(uint32 frame, uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose);
    void insertLight(uint32 frame, Light light);
    void updateCamera(uint32 frame, Camera camera);
    void reset(uint32 frame);

    TempBuffer<Light>& getLights(uint32 frame);
    TempBuffer<Transform>& getTransforms(uint32 frame);
    TempBuffer<Camera>& getCamera(uint32 frame);
    TempBuffer<DrawData>& getDrawData(uint32 frame);
    TempBuffer<Scene>& getScene(uint32 frame);

    BatchManager& getBatchManager(uint32 frame);

private:
    TempBuffer<DrawData> m_drawData[MAX_FRAME_COUNT];
    TempBuffer<Camera> m_cameras[MAX_FRAME_COUNT];
    TempBuffer<Scene> m_sceneData[MAX_FRAME_COUNT];
    TempBuffer<Light> m_lights[MAX_FRAME_COUNT];
    TempBuffer<Transform> m_transforms[MAX_FRAME_COUNT];
    
    BatchManager m_batchManagers[MAX_FRAME_COUNT];
    mmap m_meshData;
};
}