#include "SceneManager.h"

namespace spr::gfx {

SceneManager::SceneManager(){
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){        
        m_transforms[i] = TempBuffer<Transform>(MAX_DRAWS);
        m_drawData[i] = TempBuffer<DrawData>(MAX_DRAWS);
        m_lights[i] = TempBuffer<Light>(MAX_LIGHTS);
        m_cameras[i] = TempBuffer<Camera>(1);
        m_sceneData[i] = TempBuffer<Scene>(1);
    }
}
SceneManager::~SceneManager(){

}


void SceneManager::insertMesh(uint32 frame, uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose){
    // get mesh data and fill draw
    MeshInfo& meshInfo = m_meshInfo[meshId];
    DrawData draw = {
        .vertexOffset   = meshInfo.vertexOffset,  
        .materialIndex  = meshInfo.materialIndex, 
        .transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert({
            model,
            modelInvTranspose
        })
    };

    // fill out batch info, which will either initialize
    // a new batch or update an existing one
    Batch batchInfo = {
        .meshId         = meshId,
        .materialFlags  = materialFlags,
        .indexCount     = meshInfo.indexCount,
        .firstIndex     = meshInfo.firstIndex,
        .drawDataOffset = 0,
        .drawCount      = 1
    };

    m_batchManagers[frame % MAX_FRAME_COUNT].addDraw(draw, batchInfo);
}


void SceneManager::insertLight(uint32 frame, Light light){
    m_lights[frame % MAX_FRAME_COUNT].insert(light);
}


void SceneManager::updateCamera(uint32 frame, Camera camera){
    m_cameras[frame % MAX_FRAME_COUNT].insert(camera);
}


BatchManager& SceneManager::getBatchManager(uint32 frame) {
    return m_batchManagers[frame % MAX_FRAME_COUNT];
}

void SceneManager::reset(uint32 frame) {
    m_batchManagers[frame % MAX_FRAME_COUNT].reset();
    m_cameras[frame % MAX_FRAME_COUNT].reset();
    m_transforms[frame % MAX_FRAME_COUNT].reset();
    m_lights[frame % MAX_FRAME_COUNT].reset();
    m_drawData[frame % MAX_FRAME_COUNT].reset();
    m_sceneData[frame % MAX_FRAME_COUNT].reset();
}

}