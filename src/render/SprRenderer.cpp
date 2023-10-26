#include "SprRenderer.h"

#include "interface/SprWindow.h"
#include "debug/SprLog.h"
#include "resource/SprResourceManager.h"
#include "scene/SceneData.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <glm/gtc/matrix_inverse.hpp>

namespace spr {
    class SprResourceManager;
}

namespace spr {

SprRenderer::SprRenderer(SprWindow* window) : m_renderer(window), m_renderCoordinator(window){
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

void SprRenderer::loadAssets(SprResourceManager& rm){
    m_srm = &rm;
    m_sceneManager.initializeAssets(rm, &m_renderer.getDevice());
    m_renderCoordinator.initRenderers(m_sceneManager);
}


// ╔══════════════════════════════════════════════════════════════════════════╗
// ║     render                                                               ║
// ╚══════════════════════════════════════════════════════════════════════════╝
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


// ╔══════════════════════════════════════════════════════════════════════════╗
// ║     Light                                                                ║
// ╚══════════════════════════════════════════════════════════════════════════╝

void SprRenderer::insertLight(uint32 id, const gfx::Light& light){
    m_sceneManager.insertLights(m_frameId, {light});
}

void SprRenderer::insertLights(Span<uint32> ids, Span<const gfx::Light> lights){
    m_sceneManager.insertLights(m_frameId, lights);
}

void SprRenderer::updateLight(uint32 id, const gfx::Light& light){
    
}


// ╔══════════════════════════════════════════════════════════════════════════╗
// ║     Camera                                                               ║
// ╚══════════════════════════════════════════════════════════════════════════╝

void SprRenderer::updateCamera(const gfx::Camera& camera){
    m_sceneManager.updateCamera(m_frameId, {m_window->width(), m_window->height()}, camera);
}


// ╔══════════════════════════════════════════════════════════════════════════╗
// ║     Models                                                               ║
// ╚══════════════════════════════════════════════════════════════════════════╝

void SprRenderer::insertModel(uint32 id, uint32 modelId, const TransformInfo& transformInfo){
    insertModel(id, modelId, buildTransform(transformInfo));
}

void SprRenderer::insertModel(uint32 id, uint32 modelId, uint32 materialFlags, const TransformInfo& transformInfo){
    insertModel(id, modelId, materialFlags, buildTransform(transformInfo));
}

void SprRenderer::insertModel(uint32 id, uint32 modelId, const gfx::Transform& transform){
    spr::Model* model = m_srm->getData<Model>(modelId);
    Span<uint32> meshIds = model->meshIds;
    
    uint32 materialsFlags[meshIds.size()];
    for(uint32 i = 0; i < meshIds.size(); i++)
        materialsFlags[i] = m_srm->getData<Mesh>(meshIds[i])->materialFlags;

    m_sceneManager.insertMeshes(m_frameId, id, meshIds, {materialsFlags, meshIds.size()}, transform);
}

void SprRenderer::insertModel(uint32 id, uint32 modelId, uint32 materialFlags, const gfx::Transform& transform){
    spr::Model* model = m_srm->getData<Model>(modelId);
    Span<uint32> meshIds = model->meshIds;

    m_sceneManager.insertMeshes(m_frameId, id, meshIds, materialFlags, transform);
}


void SprRenderer::insertModel(uint32 id, uint32 modelId){
    spr::Model* model = m_srm->getData<Model>(modelId);
    Span<uint32> meshIds = model->meshIds;
    
    uint32 materialsFlags[meshIds.size()];
    for(uint32 i = 0; i < meshIds.size(); i++)
        materialsFlags[i] = m_srm->getData<Mesh>(meshIds[i])->materialFlags;

    m_sceneManager.insertMeshes(m_frameId, id, meshIds, {materialsFlags, meshIds.size()});
}

void SprRenderer::insertModel(uint32 id, uint32 modelId, uint32 materialFlags){
    spr::Model* model = m_srm->getData<Model>(modelId);
    Span<uint32> meshIds = model->meshIds;

    m_sceneManager.insertMeshes(m_frameId, id, meshIds, materialFlags);
}


void SprRenderer::updateModel(uint32 id, uint32 modelId, const TransformInfo& transformInfo){
    updateModel(id, modelId, buildTransform(transformInfo));
}

void SprRenderer::updateModel(uint32 id, uint32 modelId, const gfx::Transform& transform){
    spr::Model* model = m_srm->getData<Model>(modelId);
    Span<uint32> meshIds = model->meshIds;
    
    uint32 materialsFlags[meshIds.size()];
    for(uint32 i = 0; i < meshIds.size(); i++)
        materialsFlags[i] = m_srm->getData<Mesh>(meshIds[i])->materialFlags;

    m_sceneManager.updateMeshes(m_frameId, id, meshIds, {materialsFlags, meshIds.size()}, transform);
}


gfx::Transform SprRenderer::buildTransform(const TransformInfo &info){
    mat4 translation = translate(mat4(1.f), info.position);
    mat4 rotation = glm::mat4_cast(info.rotation);
    mat4 scale = glm::scale(mat4(1.f), vec3(info.scale));

    mat4 model = translation * rotation * scale;
    mat4 modelInvTranspose = inverseTranspose(model);

    return {model, modelInvTranspose};
}

}