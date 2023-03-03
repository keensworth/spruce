#pragma once

#include "scene/SceneManager.h"
#include "vulkan/VulkanRenderer.h"
#include "vulkan/resource/ResourceTypes.h"
#include "vulkan/FrameRenderer.h"



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

    uint32 m_frame = 0;

    Handle<Buffer> m_lightsBuffer;
    Handle<Buffer> m_transformBuffer;
    Handle<Buffer> m_drawDataBuffer;
    Handle<Buffer> m_cameraBuffer;
    Handle<Buffer> m_sceneBuffer;

    const uint32 MAX_DRAWS = 1<<17; // 2^18 draws

    void initBuffers();
    void initRenderers();
    void uploadResources(SceneManager& sceneManager);

private:
    // MAIN renderer
    FrameRenderer m_frameRenderer;

    // OFFSCREEN renderers
};
}