#pragma once

#include "util/BatchManager.h"
#include "util/SceneManager.h"
#include "spruce_core.h"
#include "RenderCoordinator.h"
#include "vulkan/VulkanRenderer.h"
#include "../interface/Window.h"

namespace spr::gfx {
class SprRenderer {
public:
    SprRenderer();
    ~SprRenderer();

    void uploadMeshes();

    void insertMesh(uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose);

    void insertLight();

    void updateCamera();

    void render();

    void setWindow(Window* window);
    
private:
    VulkanRenderer* m_renderer;
    VulkanResourceManager* m_rm;
    Window* m_window;

    BatchManager m_batchManager;
    SceneManager m_sceneManager;
    RenderCoordinator m_renderCoordinator;
};
}