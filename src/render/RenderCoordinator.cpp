#include "RenderCoordinator.h"

namespace spr::gfx {

RenderCoordinator::RenderCoordinator(){}
RenderCoordinator::RenderCoordinator(Window* window, VulkanRenderer* renderer, VulkanResourceManager* rm){
    m_window = window;
    m_rm = rm;
    m_renderer = renderer;
    m_frameId = 0;

    initRenderers();
}

RenderCoordinator::~RenderCoordinator(){

}


void RenderCoordinator::render(SceneManager& sceneManager){
    // begin frame
    RenderFrame& frame = m_renderer->beginFrame(m_rm);

    // upload scene data
    uploadSceneData(sceneManager);

    // begin render passes
    BatchManager& batchManager = sceneManager.getBatchManager(m_frameId);

    // PASS 1
    // PASS 2
    // PASS 3
    // ...
    // PASS N

    // render to swapchain image
    m_frameRenderer.render(batchManager);

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


void RenderCoordinator::initRenderers(){
    // setup renderers
    // TODO
}


void RenderCoordinator::onResize(){
    m_renderer->onResize();

    // get all renderpasses
    //Handle<RenderPass> passA =  m_rendererA.getRenderpass();

    // rebuild all framebuffers
    //m_rm.recreate<RenderPass>(passA, glm::uvec2(m_window->width(), m_window->height()));

}


void RenderCoordinator::destroy(){
    // teardown renderers
}

}