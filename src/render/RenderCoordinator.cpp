#include "RenderCoordinator.h"
#include "vulkan/CommandBuffer.h"
#include "vulkan/VulkanRenderer.h"
#include "vulkan/resource/ResourceTypes.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

RenderCoordinator::RenderCoordinator(Window* window){
    // initialize 
    m_window = window;
    m_rm = VulkanResourceManager();
    m_renderer = VulkanRenderer(window, &m_rm);
    m_rm.init(m_renderer.getDevice(), m_renderer.getAllocator());

    // begin command recording (for initial transfer)
    CommandBuffer& cb = m_renderer.beginCommandRecording(CommandType::TRANSFER);
    UploadHandler& uploadHandler = cb.beginTransfer();
    m_rm.setUploadHandler(&uploadHandler);

    // setup renderers
    init();

    cb.submit();
}

RenderCoordinator::~RenderCoordinator(){

}

void RenderCoordinator::init(){
    // setup renderers
    // TODO
}

void RenderCoordinator::render(SceneManager& sceneManager){
    
}

void RenderCoordinator::onResize(){
    // wait all current frames
    m_renderer.wait();

    // get all renderpasses
    //Handle<RenderPass> passA =  m_rendererA.getRenderpass();

    // rebuild all framebuffers
    //m_rm.recreate<RenderPass>(passA, glm::uvec2(m_window->width(), m_window->height()));

}

void RenderCoordinator::destroy(){

}

}