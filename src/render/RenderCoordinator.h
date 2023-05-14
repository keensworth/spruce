#pragma once

#include "renderers/UnlitMeshRenderer.h"
#include "renderers/LitMeshRenderer.h"
#include "vulkan/FrameRenderer.h"
#include "renderers/TestRenderer.h"
#include "renderers/DebugMeshRenderer.h"

namespace spr {
    class Window;
}

namespace spr::gfx{

class VulkanResourceManager;
class VulkanRenderer;
class SceneManager;

class RenderCoordinator{
public:
    RenderCoordinator();
    RenderCoordinator(Window* window);
    ~RenderCoordinator();

    void init(VulkanRenderer* renderer, VulkanResourceManager* rm);

    void initRenderers(SceneManager& sceneManager);
    void render(SceneManager& sceneManager);
    
    void onResize();
    void destroy();

private: 
    // non-owning
    Window* m_window;
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;
    uint32 m_frameId;

    bool m_sceneInitialized = false;
    
    void uploadSceneData(SceneManager& sceneManager);

private:
    // MAIN renderer
    FrameRenderer m_frameRenderer;

    // OFFSCREEN renderers
    TestRenderer m_testRenderer;
    DebugMeshRenderer m_debugMeshRenderer;
    UnlitMeshRenderer m_unlitMeshRenderer;
    LitMeshRenderer m_litMeshRenderer;
};
}