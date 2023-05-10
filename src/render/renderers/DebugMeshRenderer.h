#pragma once

#include "../vulkan/VulkanRenderer.h"
#include "../vulkan/resource/ResourceTypes.h"
#include "../vulkan/resource/VulkanResourceManager.h"
#include "../scene/BatchManager.h"
#include "../vulkan/resource/ResourceFlags.h"
#include "../../debug/SprLog.h"
#include "../scene/Material.h"

namespace spr::gfx {
class DebugMeshRenderer {
public:
    DebugMeshRenderer(){}
    DebugMeshRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
    }
    ~DebugMeshRenderer(){}


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

        // color attachment
        m_attachment = m_rm ->create<TextureAttachment>({
            .textureLayout = {
                .dimensions = m_dim,
                .format = Flags::Format::RGBA8_UNORM,
                .usage = Flags::ImageUsage::IU_COLOR_ATTACHMENT |
                         Flags::ImageUsage::IU_SAMPLED          
            }
        });
        // depth attachment
        m_depthAttachment = m_rm ->create<TextureAttachment>({
            .textureLayout = {
                .dimensions = m_dim,
                .format = Flags::Format::D32_SFLOAT,
                .usage = Flags::ImageUsage::IU_DEPTH_STENCIL_ATTACHMENT
            }
        });

        // render pass
        m_renderPassLayout = m_rm->create<RenderPassLayout>({
            .depthAttachmentFormat = Flags::D32_SFLOAT,
            .colorAttatchmentFormats = {Flags::RGBA8_UNORM},
            .subpass = {
                .depthAttachment = 1,
                .colorAttachments = {0}
            }
        });
        m_renderPass = m_rm->create<RenderPass>({
            .dimensions = m_dim,
            .layout = m_renderPassLayout,
            .depthAttachment = {
                .texture = m_depthAttachment,
                .finalLayout = Flags::ImageLayout::DEPTH_STENCIL_ATTACHMENT
            },
            .colorAttachments = {
                {
                    .texture = m_attachment,
                    .finalLayout = Flags::ImageLayout::SHADER_READ_ONLY
                }
            }
        });

        // shader
        m_shader = m_rm->create<Shader>({
            .vertexShader   = {.path = "../data/shaders/spv/debug_mesh.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/debug_mesh.frag.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { frameDescSetLayout},
                { }, // unused
                { }  // unused
            },
            .graphicsState = {
                .depthTest = Flags::Compare::LESS_OR_EQUAL,
                .renderPass = m_renderPass,
            }
        });
    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(0.45098f,0.52549f,0.47058f,1.f));
        
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

    Handle<TextureAttachment> getAttachment(){
        return m_attachment;
    }


    void destroy(){
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        m_rm->remove<TextureAttachment>(m_depthAttachment);
        m_rm->remove<TextureAttachment>(m_attachment);
        SprLog::info("[DebugMeshRenderer] [destroy] destroyed...");
    }

private: // owning
    Handle<TextureAttachment> m_attachment;
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