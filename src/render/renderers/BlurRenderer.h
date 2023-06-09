#pragma once

#include "render/vulkan/VulkanRenderer.h"
#include "render/vulkan/resource/ResourceTypes.h"
#include "render/vulkan/resource/VulkanResourceManager.h"
#include "render/scene/BatchManager.h"
#include "debug/SprLog.h"
#include "interface/SprWindow.h"
#include "vulkan/resource/ResourceFlags.h"



namespace spr::gfx {
class BlurRenderer {
public:
    BlurRenderer(){}
    BlurRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
    }
    ~BlurRenderer(){}


    void init(
        Handle<DescriptorSet> globalDescriptorSet, 
        Handle<DescriptorSetLayout> globalDescSetLayout,
        Handle<DescriptorSet> frameDescSets[MAX_FRAME_COUNT],
        Handle<DescriptorSetLayout> frameDescSetLayout,
        Handle<TextureAttachment> input)
    {
        m_globalDescriptorSet = globalDescriptorSet;
        m_globalDescriptorSetLayout = globalDescSetLayout;
        for (uint32 i = 0; i < MAX_FRAME_COUNT; i++)
            m_frameDescSets[i] = frameDescSets[i];
        m_frameDescSetLayout = frameDescSetLayout;

        // color attachment
        m_attachmentX = m_rm ->create<TextureAttachment>({
            .textureLayout = {
                .dimensions = m_dim,
                .format = Flags::Format::RGBA8_UNORM,
                .usage = Flags::ImageUsage::IU_COLOR_ATTACHMENT |
                         Flags::ImageUsage::IU_SAMPLED          
            }
        });
        m_attachmentY = m_rm ->create<TextureAttachment>({
            .textureLayout = {
                .dimensions = m_dim,
                .format = Flags::Format::RGBA8_UNORM,
                .usage = Flags::ImageUsage::IU_COLOR_ATTACHMENT |
                         Flags::ImageUsage::IU_SAMPLED          
            }
        });

        // render pass (X or horizontal blur pass)
        m_renderPassLayout = m_rm->create<RenderPassLayout>({
            .colorAttatchmentFormats = {Flags::RGBA8_UNORM},
            .subpass = {
                .colorAttachments = {0}
            }
        });
        m_renderPass = m_rm->create<RenderPass>({
            .dimensions = m_dim,
            .layout = m_renderPassLayout,
            .colorAttachments = {
                {
                    .texture = m_attachmentX,
                    .finalLayout = Flags::ImageLayout::READ_ONLY
                }
            }
        });
        m_framebufferY = m_rm->create<Framebuffer>({
            .dimensions = m_dim,
            .renderPass = m_renderPass,
            .colorAttachments = {
                {
                    .texture = m_attachmentY,
                    .finalLayout = Flags::ImageLayout::READ_ONLY
                }
            }
        });


        // descriptor set layout
        m_descriptorSetLayout = m_rm->create<DescriptorSetLayout>({
            .textures = {
                {.binding = 0}
            }
        });

        // shader (X or horizontal blur pass)
        m_shader = m_rm->create<Shader>({
            .vertexShader = {.path = "../data/shaders/spv/blur.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/blur.frag.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { frameDescSetLayout },
                { m_descriptorSetLayout },
                { } // unused
            },
            .graphicsState = {
                .depthTestEnabled = false,
                .renderPass = m_renderPass,
            }
        });

        // descriptor set
        m_descriptorSet = m_rm->create<DescriptorSet>({
            .textures = {
                {
                    .attachment = input,
                    .layout = Flags::ImageLayout::READ_ONLY
                }
            },
            .layout = m_descriptorSetLayout
        });
    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        // horizontal pass
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(1.f,1.f,1.f,1.f));
        passRenderer.drawSubpass({
            .shader = m_shader,
            .set0 =  m_globalDescriptorSet,
            .set1 = m_frameDescSets[m_renderer->getFrameId() % MAX_FRAME_COUNT],
            .set2 = m_descriptorSet}, 
            batchManager.getQuadBatch(), 0, 1);
        cb.endRenderPass();

        // horizontal pass
        passRenderer = cb.beginRenderPass(m_renderPass, m_framebufferY, glm::vec4(1.f,1.f,1.f,1.f));
        passRenderer.drawSubpass({
            .shader = m_shader,
            .set0 =  m_globalDescriptorSet,
            .set1 = m_frameDescSets[m_renderer->getFrameId() % MAX_FRAME_COUNT],
            .set2 = m_descriptorSet}, 
            batchManager.getQuadBatch(), 0, 2);
        cb.endRenderPass();
    }


    Handle<RenderPass> getRenderPass(){
        return m_renderPass;
    }

    Handle<TextureAttachment> getAttachment(){
        return m_attachmentY;
    }

    Handle<Shader> getShader(){
        return m_shader;
    }

    void destroy(){
        m_rm->remove<DescriptorSet>(m_descriptorSet);
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<DescriptorSetLayout>(m_descriptorSetLayout);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        m_rm->remove<TextureAttachment>(m_attachmentX);
        m_rm->remove<TextureAttachment>(m_attachmentY);
        SprLog::info("[BlurRenderer] [destroy] destroyed...");
    }

private: // owning
    Handle<TextureAttachment> m_attachmentX;
    Handle<TextureAttachment> m_attachmentY;
    Handle<Framebuffer> m_framebufferY;
    Handle<RenderPassLayout> m_renderPassLayout;
    Handle<RenderPass> m_renderPass;
    Handle<DescriptorSetLayout> m_descriptorSetLayout;
    Handle<DescriptorSet> m_descriptorSet;
    Handle<Shader> m_shader;


private: // non-owning
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;
    glm::uvec3 m_dim;

    Handle<DescriptorSet> m_globalDescriptorSet;
    Handle<DescriptorSetLayout> m_globalDescriptorSetLayout;
    Handle<DescriptorSet> m_frameDescSets[MAX_FRAME_COUNT];
    Handle<DescriptorSetLayout> m_frameDescSetLayout;
};
}