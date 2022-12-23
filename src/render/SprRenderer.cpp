#include "SprRenderer.h"
#include "util/SceneManager.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

SprRenderer::SprRenderer(){
    m_renderer = new VulkanRenderer();
    m_rm = new VulkanResourceManager();
    m_renderCoordinator = RenderCoordinator();
    m_batchManager = BatchManager();
    m_sceneManager = SceneManager();
}
SprRenderer::~SprRenderer(){
    m_renderCoordinator.destroy();
    m_batchManager.destroy(m_rm);
    delete m_rm;
    delete m_renderer;
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
    m_renderCoordinator.render(&m_sceneManager);
    m_sceneManager.reset();
}

void SprRenderer::setWindow(Window* window){
    m_window = window;
}

}