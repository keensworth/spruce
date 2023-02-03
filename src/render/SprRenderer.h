#pragma once

#include "scene/BatchManager.h"
#include "scene/SceneManager.h"
#include "spruce_core.h"
#include "RenderCoordinator.h"
#include "vulkan/VulkanRenderer.h"
#include "../interface/Window.h"

namespace spr::gfx {
class SprRenderer {
public:
    SprRenderer(Window* window);
    ~SprRenderer();

    void uploadMeshes();

    void insertMesh(uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose);

    void insertLight();

    void updateCamera();

    void render();
    
private:
    Window* m_window;

    SceneManager m_sceneManager;
    RenderCoordinator m_renderCoordinator;
};
}