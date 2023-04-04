#include "RenderCoordinator.h"
#include "vulkan/FrameRenderer.h"
#include "vulkan/resource/ResourceTypes.h"

namespace spr::gfx {

RenderCoordinator::RenderCoordinator(){}
RenderCoordinator::RenderCoordinator(Window* window, VulkanRenderer* renderer, VulkanResourceManager* rm){
    m_window = window;
    m_rm = rm;
    m_renderer = renderer;
    m_frameId = 0;
}

RenderCoordinator::~RenderCoordinator(){

}


void RenderCoordinator::render(SceneManager& sceneManager){
    // begin frame
    RenderFrame& frame = m_renderer->beginFrame(m_rm);
    BatchManager& batchManager = sceneManager.getBatchManager(m_frameId);

    // upload scene data
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

    // global frame resources
    if (!m_sceneInitialized){
        m_sceneInitialized = true;   
        sceneManager.uploadGlobalResources(uploadHandler);
    }

    // per-frame scene resources
    sceneManager.uploadPerFrameResources(uploadHandler, m_frameId);

    uploadHandler.submit();
}

void RenderCoordinator::initRenderers(SceneManager& sceneManager){
    // setup renderers
    glm::uvec3 windowDim = {m_window->height(), m_window->width(), 1};

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