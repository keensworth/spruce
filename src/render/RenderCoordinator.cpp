#include "RenderCoordinator.h"
#include "renderers/DebugMeshRenderer.h"
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
        //m_testRenderer.render(offscreenCB, batchManager);
        //m_debugMeshRenderer.render(offscreenCB, batchManager);
        //m_unlitMeshRenderer.render(offscreenCB, batchManager);
        m_litMeshRenderer.render(offscreenCB, batchManager);
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
    glm::uvec3 windowDim = {m_window->width(), m_window->height(), 1};

    // offscreen renderers
    m_testRenderer = TestRenderer(*m_rm, *m_renderer, windowDim);
    m_testRenderer.init(
        sceneManager.getGlobalDescriptorSet(),
        sceneManager.getGlobalDescriptorSetLayout());

    m_debugMeshRenderer = DebugMeshRenderer(*m_rm, *m_renderer, windowDim);
    m_debugMeshRenderer.init(
        sceneManager.getGlobalDescriptorSet(),
        sceneManager.getGlobalDescriptorSetLayout(),
        sceneManager.getPerFrameDescriptorSets(),
        sceneManager.getPerFrameDescriptorSetLayout());

    m_unlitMeshRenderer = UnlitMeshRenderer(*m_rm, *m_renderer, windowDim);
    m_unlitMeshRenderer.init(
        sceneManager.getGlobalDescriptorSet(),
        sceneManager.getGlobalDescriptorSetLayout(),
        sceneManager.getPerFrameDescriptorSets(),
        sceneManager.getPerFrameDescriptorSetLayout());

    m_litMeshRenderer = LitMeshRenderer(*m_rm, *m_renderer, windowDim);
    m_litMeshRenderer.init(
        sceneManager.getGlobalDescriptorSet(),
        sceneManager.getGlobalDescriptorSetLayout(),
        sceneManager.getPerFrameDescriptorSets(),
        sceneManager.getPerFrameDescriptorSetLayout());

    // swapchain renderer
    m_frameRenderer = FrameRenderer(*m_rm, *m_renderer, windowDim);
    m_frameRenderer.init(
        m_renderer->getDisplay().getImageViews(), 
        sceneManager.getGlobalDescriptorSet(),
        sceneManager.getGlobalDescriptorSetLayout());
    //m_frameRenderer.setInput(m_testRenderer.getAttachment());
    //m_frameRenderer.setInput(m_debugMeshRenderer.getAttachment());
    // m_frameRenderer.setInput(m_unlitMeshRenderer.getAttachment());
    m_frameRenderer.setInput(m_litMeshRenderer.getAttachment());

    
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
    m_testRenderer.destroy();
    m_debugMeshRenderer.destroy();
    m_unlitMeshRenderer.destroy();
    m_litMeshRenderer.destroy();
}

}