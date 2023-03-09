#include "FrameRenderer.h"
#include "resource/ResourceFlags.h"
#include "resource/ResourceTypes.h"

namespace spr::gfx {

FrameRenderer::FrameRenderer(){}


FrameRenderer::FrameRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions){
    m_rm = &rm;
    m_renderer = &renderer;
    m_dim = dimensions;
}

FrameRenderer::~FrameRenderer(){

}


void FrameRenderer::init(std::vector<VkImageView>& swapchainImageViews, Handle<TextureAttachment> input){
    VkFormat displayFormat = m_renderer->getDisplay().getSwapchainFormat();

    // render pass
    m_renderPassLayout = m_rm->create<RenderPassLayout>(RenderPassLayoutDesc{
        .colorAttatchmentFormats = {displayFormat},
        .subpass = {
            .colorAttachments = {0}
        }
    });

    m_renderPass = m_rm->create<RenderPass>(RenderPassDesc{
        .dimensions = m_dim,
        .layout = m_renderPassLayout,
        .colorAttachments = {
            {
                .swapchainImageViews = swapchainImageViews,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            }
        }
    });

    // descriptor set layout
    m_descriptorSetLayout = m_rm->create<DescriptorSetLayout>(DescriptorSetLayoutDesc{
        .textures = {
            {.binding = 0}
        }
    });

    // shader
    m_shader = m_rm->create<Shader>(ShaderDesc{
        .vertexShader = {.path = ""},
        .fragmentShader = {.path = ""},
        .descriptorSets = {
            { }, // unused
            { }, // unused
            { }, // unused
            { m_descriptorSetLayout }
        },
        .graphicsState = {
            .renderPass = m_renderPass,
        }
    });

    // descriptor set
    TextureAttachment* attachment = m_rm->get<TextureAttachment>(input);
    m_descriptorSet = m_rm->create<DescriptorSet>(DescriptorSetDesc{
        .textures = {
            {.textures = attachment->textures}
        },
        .layout = m_descriptorSetLayout
    });
}


void FrameRenderer::render(){

}


void FrameRenderer::destroy(){

}

}