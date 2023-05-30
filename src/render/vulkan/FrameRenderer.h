#pragma once

#include "VulkanRenderer.h"
#include "resource/ResourceTypes.h"
#include "resource/VulkanResourceManager.h"
#include "render/scene/BatchManager.h"
#include "resource/ResourceFlags.h"
#include "debug/SprLog.h"
#include "interface/SprWindow.h"



namespace spr::gfx {
class FrameRenderer {
public:
    FrameRenderer(){}
    FrameRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, SprWindow* window, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
        m_window = window;
    }
    ~FrameRenderer(){}


    void init(
        std::vector<VkImageView>& swapchainImageViews, 
        Handle<DescriptorSet> globalDescriptorSet, 
        Handle<DescriptorSetLayout> globalDescSetLayout)
    {
        m_globalDescriptorSet = globalDescriptorSet;
        m_globalDescriptorSetLayout = globalDescSetLayout;

        // render pass
        VkFormat displayFormat = m_renderer->getDisplay().getSwapchainFormat();
        m_renderPassLayout = m_rm->create<RenderPassLayout>({
            .colorAttatchmentFormats = {displayFormat},
            .subpass = {
                .colorAttachments = {0}
            }
        });

        m_renderPass = m_rm->create<RenderPass>({
            .dimensions = m_dim,
            .layout = m_renderPassLayout,
            .colorAttachments = {
                {
                    .swapchainImageViews = swapchainImageViews,
                    .finalLayout = Flags::ImageLayout::PRESENT
                }
            }
        });

        // descriptor set layout
        m_descriptorSetLayout = m_rm->create<DescriptorSetLayout>({
            .textures = {
                {.binding = 0}
            }
        });

        // shader
        m_shader = m_rm->create<Shader>({
            .vertexShader = {.path = "../data/shaders/spv/copy_swapchain.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/copy_swapchain.frag.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { }, // unused
                { m_descriptorSetLayout },
                { } // unused
            },
            .graphicsState = {
                .renderPass = m_renderPass,
            }
        });
    }


    void setInput(Handle<TextureAttachment> input){
        if (input == m_input)
            return;

        m_descriptorSet = m_rm->create<DescriptorSet>({
            .textures = {
                {
                    .attachment = input,
                    .layout = Flags::ImageLayout::READ_ONLY
                }
            },
            .layout = m_descriptorSetLayout
        });

        m_hasInput = true;
    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        if (!m_hasInput)
            SprLog::error("[FrameRenderer] [render] no input TextureAttachment specified");

        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(1.f,0.f,1.f,1.f));
        passRenderer.drawSubpass({
            .shader = m_shader,
            .set0 =  m_globalDescriptorSet,
            .set2 = m_descriptorSet}, 
            batchManager.getQuadBatch(), 0, 0);
        cb.endRenderPass();
    }


    Handle<RenderPass> getRenderPass(){
        return m_renderPass;
    }


    void destroy(){
        m_rm->remove<DescriptorSet>(m_descriptorSet);
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<DescriptorSetLayout>(m_descriptorSetLayout);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        SprLog::info("[FrameRenderer] [destroy] destroyed...");
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
    SprWindow* m_window;
    glm::uvec3 m_dim;

    Handle<TextureAttachment> m_input;
    bool m_hasInput = false;

    Handle<DescriptorSet> m_globalDescriptorSet;
    Handle<DescriptorSetLayout> m_globalDescriptorSetLayout;
};
}