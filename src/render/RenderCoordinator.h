#pragma once

#include "scene/SceneManager.h"
#include "vulkan/VulkanRenderer.h"



namespace spr::gfx{

class RenderCoordinator{
public:
    RenderCoordinator(Window* window);
    ~RenderCoordinator();

    void render(SceneManager& sceneManager);
    void onResize();
    void destroy();

private:
    Window* m_window;

    VulkanResourceManager m_rm;
    VulkanRenderer m_renderer;

    void init();
};
}