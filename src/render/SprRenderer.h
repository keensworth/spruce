#pragma once

#include "spruce_core.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include "vulkan/VulkanRenderer.h"
#include "SceneManager.h"
#include "RenderCoordinator.h"
#include "../core/util/Span.h"


namespace spr {
    class Window;
}

namespace spr::gfx {
    
struct Transform;
struct Light;
struct Camera;

struct TransformInfo {
    glm::vec3 position;
    glm::quat rotation;
    float scale;
};

class SprRenderer {
public:
    SprRenderer(Window* window);
    ~SprRenderer();

    void loadAssets(SprResourceManager& rm);

    void insertMesh(uint32 meshId, uint32 materialFlags, const Transform& transform);
    void insertMeshes(spr::Span<uint32> meshIds, spr::Span<uint32> materialFlags, spr::Span<const Transform> transforms);
    void insertMeshes(spr::Span<uint32> meshIds, uint32 materialFlags, spr::Span<const Transform> transforms);

    void insertMesh(uint32 meshId, uint32 materialFlags, const TransformInfo& transformInfo);
    void insertMeshes(spr::Span<uint32> meshIds, spr::Span<uint32> materialFlags, spr::Span<const TransformInfo> transformInfo);
    void insertMeshes(spr::Span<uint32> meshIds, uint32 materialFlags, spr::Span<const TransformInfo> transformInfo);

    void insertLight(const Light& light);
    void insertLights(spr::Span<const Light> lights);

    void updateCamera(const Camera& camera);

    void render();
    
private:
    VulkanRenderer m_renderer;
    VulkanResourceManager m_rm;
        
    SceneManager m_sceneManager;
    RenderCoordinator m_renderCoordinator;
    uint32 m_frameId;    

    // non-owning
    Window* m_window;
};
}