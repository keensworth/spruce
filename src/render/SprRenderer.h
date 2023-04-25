#pragma once

#include "spruce_core.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include "vulkan/VulkanRenderer.h"
#include "SceneManager.h"
#include "RenderCoordinator.h"

namespace spr {
    class Window;
}

namespace spr::gfx {
    
struct Transform;
struct Light;
struct Camera;

class SprRenderer {
public:
    SprRenderer(Window* window);
    ~SprRenderer();

    void loadAssets(SprResourceManager& rm);

    void insertMesh(uint32 meshId, uint32 materialFlags, Transform& transform);
    void insertLight(Light& light);
    void updateCamera(Camera& camera);

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