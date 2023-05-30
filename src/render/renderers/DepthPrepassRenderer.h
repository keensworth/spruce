#pragma once

#include "../vulkan/VulkanRenderer.h"
#include "../vulkan/resource/ResourceTypes.h"
#include "../vulkan/resource/VulkanResourceManager.h"
#include "../scene/BatchManager.h"
#include "../vulkan/resource/ResourceFlags.h"
#include "../../debug/SprLog.h"
#include "../scene/Material.h"

namespace spr::gfx {
class DepthPrepassRenderer {
public:
    DepthPrepassRenderer(){}
    DepthPrepassRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
    }
    ~DepthPrepassRenderer(){}


    void init(
        Handle<DescriptorSet> globalDescSet, 
        Handle<DescriptorSetLayout> globalDescSetLayout,
        Handle<DescriptorSet> frameDescSets[MAX_FRAME_COUNT],
        Handle<DescriptorSetLayout> frameDescSetLayout)
    {
        m_globalDescSet = globalDescSet;
        m_globalDescSetLayout = globalDescSetLayout;
        for (uint32 i = 0; i < MAX_FRAME_COUNT; i++)
            m_frameDescSets[i] = frameDescSets[i];
        m_frameDescSetLayout = frameDescSetLayout;

        // depth attachment
        m_depthAttachment = m_rm ->create<TextureAttachment>({
            .textureLayout = {
                .dimensions = m_dim,
                .format = Flags::Format::D32_SFLOAT,
                .usage = Flags::ImageUsage::IU_DEPTH_STENCIL_ATTACHMENT | 
                         Flags::ImageUsage::IU_SAMPLED,
                .sampler = { .addressing = Flags::Wrap::CLAMP_TO_BORDER}
            }
        });

        // render pass
        m_renderPassLayout = m_rm->create<RenderPassLayout>({
            .depthAttachmentFormat = Flags::D32_SFLOAT,
            .subpass = {
                .depthAttachment = 1,
            }
        });
        m_renderPass = m_rm->create<RenderPass>({
            .dimensions = m_dim,
            .layout = m_renderPassLayout,
            .depthAttachment = {
                .texture = m_depthAttachment,
                .finalLayout = Flags::ImageLayout::READ_ONLY
            }
        });

        // shader
        m_shader = m_rm->create<Shader>({
            .vertexShader   = {.path = "../data/shaders/spv/depth_pass.vert.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { frameDescSetLayout},
                { }, // unused
                { }  // unused
            },
            .graphicsState = {
                .depthTest = Flags::Compare::GREATER_OR_EQUAL,
                .renderPass = m_renderPass,
            }
        });
    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(0.0f,0.0f,0.0f,1.f));
        
        std::vector<Batch> batches;
        batchManager.getBatches({.hasAny = MTL_ALL}, batches);

        passRenderer.drawSubpass({
            .shader = m_shader, 
            .set0 =  m_globalDescSet,
            .set1 = m_frameDescSets[m_renderer->getFrameId() % MAX_FRAME_COUNT]}, 
            batches);

        cb.endRenderPass();
    }


    Handle<RenderPass> getRenderPass(){
        return m_renderPass;
    }

    Handle<TextureAttachment> getDepthAttachment(){
        return m_depthAttachment;
    }

    Handle<Shader> getShader(){
        return m_shader;
    }


    void destroy(){
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        m_rm->remove<TextureAttachment>(m_depthAttachment);
        SprLog::info("[DepthPrepassRenderer] [destroy] destroyed...");
    }

private: // owning
    Handle<TextureAttachment> m_depthAttachment;
    Handle<RenderPassLayout> m_renderPassLayout;
    Handle<RenderPass> m_renderPass;
    Handle<Shader> m_shader;

private: // non-owning
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;
    glm::uvec3 m_dim;

    Handle<DescriptorSet> m_globalDescSet;
    Handle<DescriptorSetLayout> m_globalDescSetLayout;
    Handle<DescriptorSet> m_frameDescSets[MAX_FRAME_COUNT];
    Handle<DescriptorSetLayout> m_frameDescSetLayout;
};
}