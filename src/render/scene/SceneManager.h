#pragma once

#include "glm/fwd.hpp"
#include "spruce_core.h"
#include "BatchManager.h"
#include "../vulkan/resource/VulkanResourceManager.h"
#include <vector>
#include "SceneObjects.h"
#include "../../core/memory/TempBuffer.h"
#include "../../../external/flat_hash_map/flat_hash_map.hpp"


namespace spr::gfx {

typedef struct MeshData {
    uint32 vertexOffset;
    uint32 indexCount;
    uint32 materialIndex;
    uint32 padding;
} MeshData;

typedef ska::flat_hash_map<uint32, MeshData> mmap;

typedef struct Transform {
    glm::mat4 model;
    glm::mat4 modelInvTranspose;
} Transform;

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void insertMesh(uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose);
    void insertLight(Light light);
    void updateCamera(Camera camera);
    void reset();

    

private:
    Camera m_camera;
    std::vector<Light> m_lights;
    BatchManager m_batchManager;
    TempBuffer<Transform> m_transforms;
    
    mmap m_meshData;
};
}