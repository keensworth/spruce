#include "FrameRenderer.h"
#include "resource/ResourceFlags.h"

namespace spr::gfx {

FrameRenderer::FrameRenderer(VulkanResourceManager& rm, VkFormat displayFormat, glm::vec3 dimensions, Handle<Texture> result){

    m_renderPassLayout = rm.create<RenderPassLayout>((RenderPassLayoutDesc){
        .colorAttatchmentFormats = {displayFormat},
        .subpass = {
            .colorAttachments = {0}
        }
    });

    m_renderPass = rm.create<RenderPass>((RenderPassDesc){
        .dimensions = dimensions,
        .layout = m_renderPassLayout,
        .colorAttachments = {
            {.texture = Handle<TextureAttachment>()} // TODO: insert handle wrapper of swapchain image
        }
    });

    // TODO: need to create descriptor sets + layouts (pass in global ones?)
    //       use result tex handle to set tex binding

    m_shader = rm.create<Shader>((ShaderDesc){
        .vertexShader = {.shaderPath = ""},
        .fragmentShader = {.shaderPath = ""},
        .descriptorSets = {},
        .graphicsState = {
            .renderPass = m_renderPass
        }
    });
}

FrameRenderer::~FrameRenderer(){

}

void FrameRenderer::render(){

}

}