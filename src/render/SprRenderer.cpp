#include "SprRenderer.h"
#include "util/SceneManager.h"

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
    DrawData draw {
        .vertexOffset = 0,   // rm->getOffset(meshId)
        .materialIndex = 0,  // rm->getMaterial(meshId)
        .transformIndex = 0  // rm->addTransform({model, modelInvTranspose})
    };
    m_batchManager.addDraw(draw, meshId, materialFlags);
}

void SprRenderer::insertLight(){
    m_sceneManager.insertLight();
}

void SprRenderer::updateCamera(){
    m_sceneManager.updateCamera();
}

void SprRenderer::render(){
    m_renderCoordinator.render(&m_batchManager);
    m_sceneManager.reset();
    m_batchManager.reset(m_rm);
}

void SprRenderer::setWindow(Window* window){
    m_window = window;
}

}