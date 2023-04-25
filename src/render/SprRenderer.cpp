#include "SprRenderer.h"

#include "../interface/Window.h"
#include "../debug/SprLog.h"

namespace spr {
    class SprResourceManager;
}

namespace spr::gfx {

SprRenderer::SprRenderer(Window* window) : m_renderer(window), m_renderCoordinator(window){
    m_window = window;
    m_frameId = 0;
    // init renderer and resource manager
    m_rm.init(m_renderer.getDevice(), {window->width(), window->height(),1});
    m_renderer.init(&m_rm);

    // init render coordinator and scene manager
    m_renderCoordinator.init(&m_renderer, &m_rm);
    m_sceneManager.init(m_rm);
}

SprRenderer::~SprRenderer(){
    // wait on all queues
    m_renderer.wait();
    
    // begin teardown
    m_sceneManager.destroy();
    m_renderCoordinator.destroy();
    m_renderer.cleanup();
    m_rm.destroy();
    m_renderer.destroy();
    SprLog::info("[SprRenderer] [destroy] destroyed...");
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
    m_sceneManager.initializeAssets(rm, &m_renderer.getDevice());
    m_renderCoordinator.initRenderers(m_sceneManager);
}

}