#include "SceneManager.h"

namespace spr::gfx {

SceneManager::SceneManager(){

}
SceneManager::~SceneManager(){

}

void SceneManager::insertMesh(uint32 frame, uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose){
    // get mesh data and fill draw
    MeshData& meshData = m_meshData[meshId];
    DrawData draw {
        .vertexOffset = meshData.vertexOffset,  
        .indexCount = meshData.indexCount,
        .materialIndex = meshData.materialIndex, 
        .transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert({
            model,
            modelInvTranspose
        })
    };

    // store and batch draw
    m_batchManagers[frame % MAX_FRAME_COUNT].addDraw(draw, meshId, materialFlags);
}

void SceneManager::insertLight(uint32 frame, Light light){
    m_lights[frame % MAX_FRAME_COUNT].insert(light);
}

void SceneManager::updateCamera(uint32 frame, Camera camera){
    m_cameras[frame % MAX_FRAME_COUNT].insert(camera);

}

TempBuffer<Light>& SceneManager::getLights(uint32 frame) {
    return m_lights[frame % MAX_FRAME_COUNT];
}

TempBuffer<Transform>& SceneManager::getTransforms(uint32 frame) {
return m_transforms[frame % MAX_FRAME_COUNT];
}

TempBuffer<Camera>& SceneManager::getCamera(uint32 frame) {
return m_cameras[frame % MAX_FRAME_COUNT];
}

TempBuffer<Scene>& SceneManager::getScene(uint32 frame) {
return m_sceneData[frame % MAX_FRAME_COUNT];
}

BatchManager& SceneManager::getBatchManager(uint32 frame) {
return m_batchManagers[frame % MAX_FRAME_COUNT];
}

void SceneManager::reset(uint32 frame) {
    m_batchManagers[frame % MAX_FRAME_COUNT].reset();
    m_transforms[frame % MAX_FRAME_COUNT].reset();
    m_lights[frame % MAX_FRAME_COUNT].reset();
    m_cameras[frame % MAX_FRAME_COUNT].reset();
    m_sceneData[frame % MAX_FRAME_COUNT].reset();
}

}