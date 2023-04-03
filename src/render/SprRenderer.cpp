#include "SprRenderer.h"
#include "vulkan/resource/VulkanResourceManager.h"


namespace spr::gfx {

SprRenderer::SprRenderer(Window* window){
    m_window = window;
    m_frameId = 0;

    // init renderer and resource manager
    m_rm = VulkanResourceManager();
    m_renderer = VulkanRenderer(window);
    m_rm.init(m_renderer.getDevice());
    m_renderer.init(&m_rm);

    // init render coordinator and scene manager
    m_renderCoordinator = RenderCoordinator(window, &m_renderer, &m_rm);
    m_sceneManager = SceneManager(m_rm);
}

SprRenderer::~SprRenderer(){
    // wait on all queues
    m_renderer.wait();
    
    // begin teardown
    m_sceneManager.destroy();
    m_renderCoordinator.destroy();
    m_renderer.destroy();
    m_rm.destroy();
}


void SprRenderer::render(){
    // check for valid presentation
    if (m_window->isMinimzed())
        return;
    if (m_window->resized()){
        m_renderCoordinator.onResize();
        m_window->resizeHandled();
        return;
    }

    // render the scene
    m_renderCoordinator.render(m_sceneManager);
    m_sceneManager.reset(m_frameId);
    m_frameId = m_renderer.getFrameId();
}


void SprRenderer::insertMesh(uint32 meshId, uint32 materialFlags, Transform& transform){
    m_sceneManager.insertMesh(m_frameId, meshId, materialFlags, transform);
}

void SprRenderer::insertLight(Light& light){
    m_sceneManager.insertLight(m_frameId, light);
}

void SprRenderer::updateCamera(Camera& camera){
    m_sceneManager.updateCamera(m_frameId, camera);
}


void SprRenderer::loadAssets(SprResourceManager& rm){
    m_sceneManager.initializeAssets(rm);
}

}