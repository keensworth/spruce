#pragma once

#include "BatchManager.h"
#include "spruce_core.h"
#include "RenderCoordinator.h"
#include "vulkan/VulkanRenderer.h"
#include "../interface/Window.h"

namespace spr::gfx{
class SprRenderer{
public:
    SprRenderer(){
        m_renderer = new VulkanRenderer();
        m_rm = new VulkanResourceManager();
        m_renderCoordinator = RenderCoordinator();
        m_batchManager = BatchManager();
    }
    ~SprRenderer(){
        m_renderCoordinator.destroy();
        m_batchManager.destroy(m_rm);
        delete m_rm;
        delete m_renderer;
    }

    void uploadMeshes() {}

    void insertMesh(uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose){
        DrawData draw{
            .vertexOffset = 0,   // rm->getOffset(meshId)
            .materialIndex = 0,  // rm->getMaterial(meshId)
            .transformIndex = 0  // rm->addTransform({model, modelInvTranspose})
        };
        m_batchManager.addDraw(draw, meshId, materialFlags);
    }

    void insertLight(){
        m_renderCoordinator.insertLight();
    }

    void updateCamera(){
        m_renderCoordinator.updateCamera();
    }

    void render(){
        m_renderCoordinator.render(&m_batchManager);
        m_batchManager.reset(m_rm);
    }

    void setWindow(Window* window){
        m_window = window;
    }
    
private:
    VulkanRenderer* m_renderer;
    VulkanResourceManager* m_rm;
    Window* m_window;

    BatchManager m_batchManager;
    RenderCoordinator m_renderCoordinator;
};
}