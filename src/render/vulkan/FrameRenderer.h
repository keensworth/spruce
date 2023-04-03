#pragma once

#include "VulkanRenderer.h"
#include "resource/ResourceTypes.h"
#include "resource/VulkanResourceManager.h"
#include "../scene/BatchManager.h"
#include "resource/ResourceFlags.h"

namespace spr::gfx {
class FrameRenderer {
public:
    FrameRenderer(){}
    FrameRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
    }
    ~FrameRenderer(){}


    void init(std::vector<VkImageView>& swapchainImageViews, Handle<TextureAttachment> input){
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

    void render(BatchManager& batchManager){
        CommandBuffer& cb = m_renderer->beginGraphicsCommands(CommandType::MAIN);
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(1.0f,0.f,0.f,1.f));
        
        passRenderer.drawSubpass({.shader = m_shader, .set3 = m_descriptorSet}, batchManager.getQuadBatch());

        cb.endRenderPass();
        cb.submit();
    }

    void destroy(){
        
    }

private: // owning
    Handle<RenderPassLayout> m_renderPassLayout;
    Handle<RenderPass> m_renderPass;
    Handle<DescriptorSetLayout> m_descriptorSetLayout;
    Handle<DescriptorSet> m_descriptorSet;
    Handle<Shader> m_shader;

private: // non-owning
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;
    glm::uvec3 m_dim;
};
}