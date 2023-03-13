#pragma once

#include "SceneManager.h"
#include "vulkan/VulkanRenderer.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include "vulkan/FrameRenderer.h"


namespace spr::gfx{

class RenderCoordinator{
public:
    RenderCoordinator();
    RenderCoordinator(Window* window, VulkanRenderer* renderer, VulkanResourceManager* rm);
    ~RenderCoordinator();

    void render(SceneManager& sceneManager);
    void onResize();
    void destroy();

private:
    Window* m_window;
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;

    uint32 m_frameId = 0;
    
    void initRenderers();

private:
    // MAIN renderer
    FrameRenderer m_frameRenderer;

    // OFFSCREEN renderers
};
}