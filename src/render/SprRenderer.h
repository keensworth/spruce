#pragma once

#include "SceneManager.h"
#include "spruce_core.h"
#include "RenderCoordinator.h"
#include "vulkan/VulkanRenderer.h"
#include "../interface/Window.h"
#include "../resource/SprResourceManager.h"

namespace spr::gfx {
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