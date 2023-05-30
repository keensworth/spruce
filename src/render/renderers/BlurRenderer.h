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
        m_renderPassLayoutX = m_rm->create<RenderPassLayout>({
            .colorAttatchmentFormats = {Flags::RGBA8_UNORM},
            .subpass = {
                .colorAttachments = {0}
            }
        });
        m_renderPassX = m_rm->create<RenderPass>({
            .dimensions = m_dim,
            .layout = m_renderPassLayoutX,
            .colorAttachments = {
                {
                    .texture = m_attachmentX,
                    .finalLayout = Flags::ImageLayout::READ_ONLY
                }
            }
        });

        // render pass (Y or vertical blur pass)
        m_renderPassLayoutY = m_rm->create<RenderPassLayout>({
            .colorAttatchmentFormats = {Flags::RGBA8_UNORM},
            .subpass = {
                .colorAttachments = {0}
            }
        });
        m_renderPassY = m_rm->create<RenderPass>({
            .dimensions = m_dim,
            .layout = m_renderPassLayoutX,
            .colorAttachments = {
                {
                    .texture = m_attachmentY,
                    .finalLayout = Flags::ImageLayout::READ_ONLY
                }
            }
        });

        // descriptor set layout
        m_descriptorSetLayoutX = m_rm->create<DescriptorSetLayout>({
            .textures = {
                {.binding = 0}
            }
        });
        m_descriptorSetLayoutY = m_rm->create<DescriptorSetLayout>({
            .textures = {
                {.binding = 0}
            }
        });

        // shader (X or horizontal blur pass)
        m_shaderX = m_rm->create<Shader>({
            .vertexShader = {.path = "../data/shaders/spv/blur.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/blur.frag.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { frameDescSetLayout },
                { m_descriptorSetLayoutX },
                { } // unused
            },
            .graphicsState = {
                .depthTestEnabled = false,
                .renderPass = m_renderPassX,
            }
        });

        // shader (Y or vertical blur pass)
        m_shaderY = m_rm->create<Shader>({
            .vertexShader = {.path = "../data/shaders/spv/blur.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/blur.frag.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { frameDescSetLayout },
                { m_descriptorSetLayoutY },
                { } // unused
            },
            .graphicsState = {
                .depthTestEnabled = false,
                .renderPass = m_renderPassY,
            }
        });

        // descriptor set
        m_descriptorSetX = m_rm->create<DescriptorSet>({
            .textures = {
                {
                    .attachment = input,
                    .layout = Flags::ImageLayout::READ_ONLY
                }
            },
            .layout = m_descriptorSetLayoutX
        });
        m_descriptorSetY = m_rm->create<DescriptorSet>({
            .textures = {
                {
                    .attachment = m_attachmentX,
                    .layout = Flags::ImageLayout::READ_ONLY
                }
            },
            .layout = m_descriptorSetLayoutY
        });
    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        // horizontal pass
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPassX, glm::vec4(1.f,1.f,1.f,1.f));
        passRenderer.drawSubpass({
            .shader = m_shaderX,
            .set0 =  m_globalDescriptorSet,
            .set1 = m_frameDescSets[m_renderer->getFrameId() % MAX_FRAME_COUNT],
            .set2 = m_descriptorSetX}, 
            batchManager.getQuadBatch(), 0, 1);
        cb.endRenderPass();

        // horizontal pass
        passRenderer = cb.beginRenderPass(m_renderPassY, glm::vec4(1.f,1.f,1.f,1.f));
        passRenderer.drawSubpass({
            .shader = m_shaderY,
            .set0 =  m_globalDescriptorSet,
            .set1 = m_frameDescSets[m_renderer->getFrameId() % MAX_FRAME_COUNT],
            .set2 = m_descriptorSetY}, 
            batchManager.getQuadBatch(), 0, 2);
        cb.endRenderPass();
    }


    Handle<RenderPass> getRenderPassX(){
        return m_renderPassX;
    }

    Handle<RenderPass> getRenderPassY(){
        return m_renderPassY;
    }

    Handle<TextureAttachment> getAttachment(){
        return m_attachmentY;
    }

    Handle<Shader> getShaderX(){
        return m_shaderX;
    }

    Handle<Shader> getShaderY(){
        return m_shaderY;
    }


    void destroy(){
        m_rm->remove<DescriptorSet>(m_descriptorSetX);
        m_rm->remove<DescriptorSet>(m_descriptorSetY);
        m_rm->remove<Shader>(m_shaderX);
        m_rm->remove<Shader>(m_shaderY);
        m_rm->remove<DescriptorSetLayout>(m_descriptorSetLayoutX);
        m_rm->remove<DescriptorSetLayout>(m_descriptorSetLayoutY);
        m_rm->remove<RenderPass>(m_renderPassX);
        m_rm->remove<RenderPass>(m_renderPassY);
        m_rm->remove<RenderPassLayout>(m_renderPassLayoutX);
        m_rm->remove<RenderPassLayout>(m_renderPassLayoutY);
        m_rm->remove<TextureAttachment>(m_attachmentX);
        m_rm->remove<TextureAttachment>(m_attachmentY);
        SprLog::info("[BlurRenderer] [destroy] destroyed...");
    }

private: // owning
    Handle<TextureAttachment> m_attachmentY;
    Handle<TextureAttachment> m_attachmentX;
    Handle<RenderPassLayout> m_renderPassLayoutX;
    Handle<RenderPassLayout> m_renderPassLayoutY;
    Handle<RenderPass> m_renderPassX;
    Handle<RenderPass> m_renderPassY;
    Handle<DescriptorSetLayout> m_descriptorSetLayoutX;
    Handle<DescriptorSetLayout> m_descriptorSetLayoutY;
    Handle<DescriptorSet> m_descriptorSetX;
    Handle<DescriptorSet> m_descriptorSetY;
    Handle<Shader> m_shaderX;
    Handle<Shader> m_shaderY;


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