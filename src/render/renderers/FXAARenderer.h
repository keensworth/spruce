#pragma once

#include "render/vulkan/VulkanRenderer.h"
#include "render/vulkan/resource/ResourceTypes.h"
#include "render/vulkan/resource/VulkanResourceManager.h"
#include "render/scene/BatchManager.h"
#include "debug/SprLog.h"
#include "interface/SprWindow.h"
#include "vulkan/resource/ResourceFlags.h"



namespace spr::gfx {
class FXAARenderer {
public:
    FXAARenderer(){}
    FXAARenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
    }
    ~FXAARenderer(){}


    void init(
        Handle<DescriptorSet> globalDescriptorSet, 
        Handle<DescriptorSetLayout> globalDescSetLayout,
        Handle<DescriptorSet> frameDescSets,
        Handle<DescriptorSetLayout> frameDescSetLayout,
        Handle<TextureAttachment> input)
    {
        m_globalDescriptorSet = globalDescriptorSet;
        m_globalDescriptorSetLayout = globalDescSetLayout;
        m_frameDescSets = frameDescSets;
        m_frameDescSetLayout = frameDescSetLayout;

        // color attachment
        m_attachment = m_rm ->create<TextureAttachment>({
            .textureLayout = {
                .dimensions = m_dim,
                .format = Flags::Format::RGBA8_UNORM,
                .usage = Flags::ImageUsage::IU_COLOR_ATTACHMENT |
                         Flags::ImageUsage::IU_SAMPLED          
            }
        });

        // render pass 
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
                    .texture = m_attachment,
                    .finalLayout = Flags::ImageLayout::READ_ONLY
                }
            }
        });

        // descriptor set layout
        m_descriptorSetLayout = m_rm->create<DescriptorSetLayout>({
            .textures = {
                {.binding = 0}, // color
            },
        });

        // shader
        m_shader = m_rm->create<Shader>({
            .vertexShader = {.path = "../data/shaders/spv/fxaa.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/fxaa.frag.spv"},
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
                },
            },
            .layout = m_descriptorSetLayout
        });
    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(1.f,1.f,1.f,1.f));
        passRenderer.drawSubpass({
            .shader = m_shader,
            .set0 =  m_globalDescriptorSet,
            .set1 = m_frameDescSets,
            .set2 = m_descriptorSet}, 
            batchManager.getQuadBatch(), 0, 1);
        cb.endRenderPass();
    }


    Handle<RenderPass> getRenderPass(){
        return m_renderPass;
    }

    Handle<Shader> getShader(){
        return m_shader;
    }

    Handle<TextureAttachment> getAttachment(){
        return m_attachment;
    }

    void updateDescriptorSet(Handle<TextureAttachment> input){
        m_rm->remove<DescriptorSet>(m_descriptorSet);

        // descriptor set
        m_descriptorSet = m_rm->create<DescriptorSet>({
            .textures = {
                {
                    .attachment = input,
                    .layout = Flags::ImageLayout::READ_ONLY
                },
            },
            .layout = m_descriptorSetLayout
        });
    }

    void destroy(){
        m_rm->remove<DescriptorSet>(m_descriptorSet);
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<DescriptorSetLayout>(m_descriptorSetLayout);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        m_rm->remove<TextureAttachment>(m_attachment);
        SprLog::info("[FXAARenderer] [destroy] destroyed...");
    }

private: // owning
    Handle<TextureAttachment> m_attachment;
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
    Handle<DescriptorSet> m_frameDescSets;
    Handle<DescriptorSetLayout> m_frameDescSetLayout;

    friend class RenderCoordinator;
};
}