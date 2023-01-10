#include "SceneManager.h"

namespace spr::gfx {

SceneManager::SceneManager(){
    m_lights.reserve(1024);
}
SceneManager::~SceneManager(){

}

void SceneManager::insertMesh(uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose){
    // get mesh data and fill draw struct
    MeshData& meshData = m_meshData[meshId];
    DrawData draw {
        .vertexOffset = meshData.vertexOffset,  
        .indexCount = meshData.indexCount,
        .materialIndex = meshData.materialIndex, 
        .transformIndex = m_transforms.insert({
            model,
            modelInvTranspose
        })
    };

    // store and batch draw
    m_batchManager.addDraw(draw, meshId, materialFlags);
}

void SceneManager::insertLight(Light light){
    m_lights.push_back(light);
}

void SceneManager::updateCamera(Camera camera){
    m_camera = camera;
}

void SceneManager::reset(){
    m_batchManager.reset();
    m_transforms.reset();
    m_lights.clear();
}

}