#include "RenderCoordinator.h"
#include "vulkan/FrameRenderer.h"
#include "vulkan/resource/ResourceTypes.h"
#include "SceneManager.h"
#include "../interface/Window.h"

namespace spr::gfx {

RenderCoordinator::RenderCoordinator(){}
RenderCoordinator::RenderCoordinator(Window* window){
    m_window = window;
    m_frameId = 0;
}

RenderCoordinator::~RenderCoordinator(){
    SprLog::info("[RenderCoordinator] [destroy] destroyed...");
}

void RenderCoordinator::init(VulkanRenderer* renderer, VulkanResourceManager* rm){
    m_rm = rm;
    m_renderer = renderer;
}


void RenderCoordinator::render(SceneManager& sceneManager){
    // begin frame
    RenderFrame& frame = m_renderer->beginFrame(m_rm);
    BatchManager& batchManager = sceneManager.getBatchManager(m_frameId);
    uploadSceneData(sceneManager);

    // offscreen renderpasses
    CommandBuffer& offscreenCB = m_renderer->beginGraphicsCommands(CommandType::OFFSCREEN);
    offscreenCB.bindIndexBuffer(sceneManager.getIndexBuffer());
    {
        // PASS 1
        // PASS 2
        // PASS 3
        // ...
        // PASS N
    }
    offscreenCB.submit();

    // render to swapchain image
    CommandBuffer& mainCB = m_renderer->beginGraphicsCommands(CommandType::MAIN);
    mainCB.bindIndexBuffer(sceneManager.getIndexBuffer());
    {
        m_frameRenderer.render(mainCB, batchManager);
    }
    mainCB.submit();

    // present result
    m_renderer->present(frame);
    m_frameId = m_renderer->getFrameId();
}


void RenderCoordinator::uploadSceneData(SceneManager& sceneManager){
    UploadHandler& uploadHandler = m_renderer->beginTransferCommands();

    // global
    if (!m_sceneInitialized){
        m_sceneInitialized = true;
        sceneManager.uploadGlobalResources(uploadHandler);
    }

    // per-frame
    sceneManager.uploadPerFrameResources(uploadHandler, m_frameId);

    uploadHandler.submit();
}

void RenderCoordinator::initRenderers(SceneManager& sceneManager){
    // setup renderers
    glm::uvec3 windowDim = {m_window->width(), m_window->height(), 1};

    // swapchain renderer
    m_frameRenderer = FrameRenderer(*m_rm, *m_renderer, windowDim);
    m_frameRenderer.init(
        m_renderer->getDisplay().getImageViews(), 
        Handle<TextureAttachment>(),
        sceneManager.getGlobalDescriptorSet(),
        sceneManager.getGlobalDescriptorSetLayout());
}


void RenderCoordinator::onResize(){
    m_renderer->onResize();

    glm::uvec2 windowDim = {m_window->width(), m_window->height()};

    // rebuild all framebuffers
    m_rm->recreate<RenderPass>(m_frameRenderer.getRenderPass(), windowDim);
}


void RenderCoordinator::destroy(){
    // teardown renderers
    m_frameRenderer.destroy();
}

}