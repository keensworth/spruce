#include "SceneManager.h"

namespace spr::gfx {

SceneManager::SceneManager(){

}
SceneManager::~SceneManager(){

}

void SceneManager::insertMesh(uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose){
    DrawData draw {
        .vertexOffset = 0,   // rm->getOffset(meshId)
        .materialIndex = 0,  // rm->getMaterial(meshId)
        .transformIndex = 0  // rm->addTransform({model, modelInvTranspose})
    };
    m_batchManager.addDraw(draw, meshId, materialFlags);
}

void SceneManager::insertLight(){
    
}

void SceneManager::updateCamera(){
    
}

}