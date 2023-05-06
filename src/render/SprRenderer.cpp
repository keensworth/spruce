#include "SprRenderer.h"

#include "../interface/Window.h"
#include "../debug/SprLog.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/gtc/matrix_inverse.hpp>

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


void SprRenderer::insertMesh(uint32 meshId, uint32 materialFlags, const Transform& transform){
    m_sceneManager.insertMeshes(m_frameId, {meshId}, {materialFlags}, {transform});
}

void SprRenderer::insertMeshes(spr::Span<uint32> meshIds, spr::Span<uint32> materialsFlags, spr::Span<const Transform> transforms){
    if (!meshIds.size() || !materialsFlags.size() || !transforms.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != transforms.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and transforms");

    m_sceneManager.insertMeshes(m_frameId, meshIds, materialsFlags, transforms);
}

void SprRenderer::insertMesh(uint32 meshId, uint32 materialFlags, const TransformInfo& transformInfo){
    glm::mat4 translation = glm::translate(glm::mat4(1.f), transformInfo.position);
    glm::mat4 rotation = glm::toMat4(transformInfo.rotation);
    glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(transformInfo.scale));

    glm::mat4 model = translation * rotation * scale;
    glm::mat4 modelInvTranspose = glm::inverseTranspose(model);

    m_sceneManager.insertMeshes(m_frameId, {meshId}, {materialFlags}, {{model, modelInvTranspose}});
}

void SprRenderer::insertMeshes(spr::Span<uint32> meshIds, spr::Span<uint32> materialsFlags, spr::Span<const TransformInfo> transformInfos){
    if (!meshIds.size() || !materialsFlags.size() || !transformInfos.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != transformInfos.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and transforms");
    
    Transform transforms[meshIds.size()];
    for (uint32 i = 0; i < meshIds.size(); i++){
        glm::mat4 translation = glm::translate(glm::mat4(1.f), transformInfos[i].position);
        glm::mat4 rotation = glm::toMat4(transformInfos[i].rotation);
        glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(transformInfos[i].scale));

        transforms[i].model = translation * rotation * scale;
        transforms[i].modelInvTranspose = glm::inverseTranspose(transforms[i].model);
    }

    m_sceneManager.insertMeshes(m_frameId, meshIds, materialsFlags, {transforms, meshIds.size()});
}

void SprRenderer::insertLight(const Light& light){
    m_sceneManager.insertLights(m_frameId, {light});
}

void SprRenderer::insertLights(spr::Span<const Light> lights){
    m_sceneManager.insertLights(m_frameId, lights);
}

void SprRenderer::updateCamera(const Camera& camera){
    m_sceneManager.updateCamera(m_frameId, {m_window->width(), m_window->height()}, camera);
}


void SprRenderer::loadAssets(SprResourceManager& rm){
    m_sceneManager.initializeAssets(rm, &m_renderer.getDevice());
    m_renderCoordinator.initRenderers(m_sceneManager);
}

}