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
void SprRenderer::insertLight(const gfx::Light& light){
    m_sceneManager.insertLights(m_frameId, {light});
}

void SprRenderer::insertLights(Span<const gfx::Light> lights){
    m_sceneManager.insertLights(m_frameId, lights);
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

void SprRenderer::insertModel(uint32 modelId, const TransformInfo& transformInfo){
    spr::Model* model = m_srm->getData<Model>(modelId);
    insertMeshes(model->meshIds, transformInfo);
}

void SprRenderer::insertModel(uint32 modelId, uint32 materialFlags, const TransformInfo& transformInfo){
    spr::Model* model = m_srm->getData<Model>(modelId);
    insertMeshes(model->meshIds, materialFlags, transformInfo);
}

void SprRenderer::insertModel(uint32 modelId, const gfx::Transform& transform){
    spr::Model* model = m_srm->getData<Model>(modelId);
    insertMeshes(model->meshIds, transform);
}

void SprRenderer::insertModel(uint32 modelId, uint32 materialFlags, const gfx::Transform& transform){
    spr::Model* model = m_srm->getData<Model>(modelId);
    insertMeshes(model->meshIds, materialFlags, transform);
}


// ╔══════════════════════════════════════════════════════════════════════════╗
// ║     Meshes                                                               ║
// ╚══════════════════════════════════════════════════════════════════════════╝

// single mesh
void SprRenderer::insertMesh(uint32 meshId, const TransformInfo& transformInfo){
    uint32 materialFlags = m_srm->getData<Mesh>(meshId)->materialFlags;
    m_sceneManager.insertMeshes(m_frameId, {meshId}, materialFlags, buildTransform(transformInfo));
}

// batch meshes, shared transform
void SprRenderer::insertMeshes(Span<uint32> meshIds, const TransformInfo& transformInfo){
    uint32 materialsFlags[meshIds.size()];
    for(uint32 i = 0; i < meshIds.size(); i++)
        materialsFlags[i] = m_srm->getData<Mesh>(meshIds[i])->materialFlags;

    m_sceneManager.insertMeshes(m_frameId, meshIds, {materialsFlags, meshIds.size()}, buildTransform(transformInfo));
}

// batch meshes
void SprRenderer::insertMeshes(Span<uint32> meshIds, Span<const TransformInfo> transformInfos){
    if (!meshIds.size() || !transformInfos.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != transformInfos.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and transformInfos");

    uint32 materialsFlags[meshIds.size()];
    for(uint32 i = 0; i < meshIds.size(); i++)
        materialsFlags[i] = m_srm->getData<Mesh>(meshIds[i])->materialFlags;

    gfx::Transform transforms[meshIds.size()];
    for(uint32 i = 0; i < meshIds.size(); i++)
        transforms[i] = buildTransform(transformInfos[i]);

    m_sceneManager.insertMeshes(m_frameId, meshIds, {materialsFlags, meshIds.size()}, {transforms, meshIds.size()});
}

// single mesh
void SprRenderer::insertMesh(uint32 meshId, uint32 materialFlags, const TransformInfo& transformInfo){
    m_sceneManager.insertMeshes(m_frameId, {meshId}, materialFlags, buildTransform(transformInfo));
}

// batch meshes, shared material, shared transform
void SprRenderer::insertMeshes(Span<uint32> meshIds, uint32 materialFlags, const TransformInfo& transformInfo){
    m_sceneManager.insertMeshes(m_frameId, meshIds, materialFlags, buildTransform(transformInfo));
}

// batch meshes, shared material
void SprRenderer::insertMeshes(Span<uint32> meshIds, uint32 materialFlags, Span<const TransformInfo> transformInfos){
    if (!meshIds.size() || !transformInfos.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != transformInfos.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and transformInfos");
    
    gfx::Transform transforms[meshIds.size()];
    for (uint32 i = 0; i < meshIds.size(); i++){
        transforms[i] = buildTransform(transformInfos[i]);
    }

    m_sceneManager.insertMeshes(m_frameId, meshIds, materialFlags, {transforms, meshIds.size()});
}

// batch meshes, shared transform
void SprRenderer::insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags, const TransformInfo& transformInfo){
    if (!meshIds.size() || !materialsFlags.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != materialsFlags.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and materialsFlags");

    m_sceneManager.insertMeshes(m_frameId, meshIds, materialsFlags, buildTransform(transformInfo));
}

// batch meshes
void SprRenderer::insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags, Span<const TransformInfo> transformInfos){
    if (!meshIds.size() || !materialsFlags.size() || !transformInfos.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != transformInfos.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and materialsFlags");
    
    gfx::Transform transforms[meshIds.size()];
    for (uint32 i = 0; i < meshIds.size(); i++){
        transforms[i] = buildTransform(transformInfos[i]);
    }

    m_sceneManager.insertMeshes(m_frameId, meshIds, materialsFlags, {transforms, meshIds.size()});
}

// single mesh
void SprRenderer::insertMesh(uint32 meshId, const gfx::Transform& transform){
    spr::Mesh* mesh = m_srm->getData<spr::Mesh>(meshId);
    m_sceneManager.insertMeshes(m_frameId, {meshId}, mesh->materialFlags, transform);
}

// batch meshes, shared transform
void SprRenderer::insertMeshes(Span<uint32> meshIds, const gfx::Transform& transform){
    uint32 materialsFlags[meshIds.size()];
    for(uint32 i = 0; i < meshIds.size(); i++)
        materialsFlags[i] = m_srm->getData<Mesh>(meshIds[i])->materialFlags;
    m_sceneManager.insertMeshes(m_frameId, meshIds, {materialsFlags, meshIds.size()}, transform);
}

// batch meshes
void SprRenderer::insertMeshes(Span<uint32> meshIds, Span<const gfx::Transform> transforms){
    if (!meshIds.size() || !transforms.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != transforms.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and transforms");

    uint32 materialsFlags[meshIds.size()];
    for(uint32 i = 0; i < meshIds.size(); i++)
        materialsFlags[i] = m_srm->getData<Mesh>(meshIds[i])->materialFlags;
    m_sceneManager.insertMeshes(m_frameId, meshIds, {materialsFlags, meshIds.size()}, transforms);
}

// single mesh
void SprRenderer::insertMesh(uint32 meshId, uint32 materialFlags, const gfx::Transform& transform){
    m_sceneManager.insertMeshes(m_frameId, {meshId}, materialFlags, transform);
}

// batch meshes, shared material, shared transform
void SprRenderer::insertMeshes(Span<uint32> meshIds, uint32 materialFlags, const gfx::Transform& transform){
    m_sceneManager.insertMeshes(m_frameId, meshIds, materialFlags, transform);
}

// batch meshes, shared material
void SprRenderer::insertMeshes(Span<uint32> meshIds, uint32 materialFlags, Span<const gfx::Transform> transforms){
    if (!meshIds.size() || !transforms.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != transforms.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and transforms");

    m_sceneManager.insertMeshes(m_frameId, meshIds, materialFlags, transforms);
}

// batch meshes, shared transform
void SprRenderer::insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags, const gfx::Transform& transform){
    if (!meshIds.size() || !materialsFlags.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != materialsFlags.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and materialsFlags");

    m_sceneManager.insertMeshes(m_frameId, meshIds, materialsFlags, transform);
}

// batch meshes
void SprRenderer::insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags, Span<const gfx::Transform> transforms){
    if (!meshIds.size() || !materialsFlags.size() || !transforms.size())
        SprLog::error("[SprRenderer] [insertMeshes] must provide at least 1 of each parameter");

    if (meshIds.size() != transforms.size())
        SprLog::error("[SprRenderer] [insertMeshes] size mismatch between meshIds and transforms");

    m_sceneManager.insertMeshes(m_frameId, meshIds, materialsFlags, transforms);
}


void SprRenderer::loadAssets(SprResourceManager& rm){
    m_srm = &rm;
    m_sceneManager.initializeAssets(rm, &m_renderer.getDevice());
    m_renderCoordinator.initRenderers(m_sceneManager);
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