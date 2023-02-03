#include "SprRenderer.h"
#include "scene/SceneManager.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

SprRenderer::SprRenderer(Window* window) : m_renderCoordinator(window){

}

SprRenderer::~SprRenderer(){
    m_renderCoordinator.destroy();
}

void SprRenderer::uploadMeshes() {}

void SprRenderer::insertMesh(uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose){
    m_sceneManager.insertMesh(meshId, materialFlags, model, modelInvTranspose);
}

void SprRenderer::insertLight(){
    m_sceneManager.insertLight();
}

void SprRenderer::updateCamera(){
    m_sceneManager.updateCamera();
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
    m_sceneManager.reset();
}


}