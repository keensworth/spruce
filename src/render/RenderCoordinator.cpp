#include "RenderCoordinator.h"
#include "vulkan/VulkanRenderer.h"
#include "vulkan/resource/ResourceTypes.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

RenderCoordinator::RenderCoordinator(Window* window) : m_renderer(VulkanRenderer(window)){
    m_window = window;
}

RenderCoordinator::~RenderCoordinator(){

}

void RenderCoordinator::render(SceneManager& sceneManager){
    
}

void RenderCoordinator::onResize(){
    // wait all current frames
    m_renderer.wait();

    // get all renderpasses
    Handle<RenderPass> passA =  m_rendererA.getRenderpass();

    // rebuild all framebuffers
    m_rm.recreate<RenderPass>(passA, glm::uvec2(m_window->width(), m_window->height()));

}

void RenderCoordinator::destroy(){

}

}