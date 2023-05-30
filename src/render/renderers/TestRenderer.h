#pragma once

#include "../vulkan/VulkanRenderer.h"
#include "../vulkan/resource/ResourceTypes.h"
#include "../vulkan/resource/VulkanResourceManager.h"
#include "../scene/BatchManager.h"
#include "../vulkan/resource/ResourceFlags.h"
#include "../../debug/SprLog.h"
#include "../scene/Material.h"

namespace spr::gfx {
class TestRenderer {
public:
    TestRenderer(){}
    TestRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
    }
    ~TestRenderer(){}


    void init(
        Handle<DescriptorSet> globalDescriptorSet, 
        Handle<DescriptorSetLayout> globalDescSetLayout)
    {
        m_globalDescriptorSet = globalDescriptorSet;
        m_globalDescriptorSetLayout = globalDescSetLayout;

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

        // shader
        m_shader = m_rm->create<Shader>({
            .vertexShader = {.path = "../data/shaders/spv/test.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/test.frag.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { }, // unused
                { }, // unused
                { }  // unused
            },
            .graphicsState = {
                .renderPass = m_renderPass,
            }
        });

    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(0.0f,1.f,0.f,1.f));
        
        passRenderer.drawSubpass(
            {.shader = m_shader, .set0 =  m_globalDescriptorSet}, 
            batchManager.getQuadBatch(), 0, 0);

        cb.endRenderPass();
    }


    Handle<RenderPass> getRenderPass(){
        return m_renderPass;
    }

    Handle<TextureAttachment> getAttachment(){
        return m_attachment;
    }

    Handle<Shader> getShader(){
        return m_shader;
    }

    void destroy(){
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        m_rm->remove<TextureAttachment>(m_attachment);
        SprLog::info("[TestRenderer] [destroy] destroyed...");
    }

private: // owning
    Handle<TextureAttachment> m_attachment;
    Handle<RenderPassLayout> m_renderPassLayout;
    Handle<RenderPass> m_renderPass;
    Handle<Shader> m_shader;

private: // non-owning
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;
    glm::uvec3 m_dim;

    Handle<DescriptorSet> m_globalDescriptorSet;
    Handle<DescriptorSetLayout> m_globalDescriptorSetLayout;
};
}