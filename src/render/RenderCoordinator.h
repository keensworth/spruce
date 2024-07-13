#pragma once

#include "renderers/BlurRenderer.h"
#include "renderers/DebugClustersRenderer.h"
#include "renderers/DebugNormalsRenderer.h"
#include "renderers/DepthPrepassRenderer.h"
#include "renderers/FXAARenderer.h"
#include "renderers/GTAORenderer.h"
#include "renderers/LightCullCompute.h"
#include "renderers/UnlitMeshRenderer.h"
#include "renderers/LitMeshRenderer.h"
#include "renderers/SkyboxRenderer.h"
#include "renderers/TestRenderer.h"
#include "renderers/DebugMeshRenderer.h"
#include "renderers/SunShadowRenderer.h"
#include "renderers/VolumetricLightRenderer.h"
#include "renderers/DebugCascadesRenderer.h"
#include "vulkan/FrameRenderer.h"
#include "vulkan/ImGuiRenderer.h"

namespace spr {
    class SprWindow;
}

namespace spr::gfx{

class VulkanResourceManager;
class VulkanRenderer;
class SceneManager;

class RenderCoordinator{
public:
    RenderCoordinator();
    RenderCoordinator(SprWindow* window);
    ~RenderCoordinator();

    void init(VulkanRenderer* renderer, VulkanResourceManager* rm);

    void initRenderers(SceneManager& sceneManager);
    void render(SceneManager& sceneManager);
    
    void onResize();
    void destroy();

private: 
    // non-owning
    SprWindow* m_window;
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;
    uint32 m_frameId;

    bool m_sceneInitialized = false;
    
    void uploadSceneData(SceneManager& sceneManager);

    void updateUI(CommandBuffer& offscreenCB);

private:
    // MAIN renderer
    FrameRenderer m_frameRenderer;

    // OFFSCREEN renderers
    TestRenderer m_testRenderer;
    DebugMeshRenderer m_debugMeshRenderer;
    DebugNormalsRenderer m_debugNormalsRenderer;
    DebugCascadesRenderer m_debugCascadesRenderer;
    DebugClustersRenderer m_debugClustersRenderer;
    DepthPrepassRenderer m_depthPrepassRenderer;
    GTAORenderer m_gtaoRenderer;
    BlurRenderer m_blurRenderer;
    FXAARenderer m_fxaaRenderer;
    SunShadowRenderer m_sunShadowRenderer;
    VolumetricLightRenderer m_volumetricLightRenderer;
    UnlitMeshRenderer m_unlitMeshRenderer;
    LitMeshRenderer m_litMeshRenderer;
    SkyboxRenderer m_skyboxRenderer;
    ImGuiRenderer m_imguiRenderer;

    // compute
    LightCullCompute m_lightCullCompute;
};
}