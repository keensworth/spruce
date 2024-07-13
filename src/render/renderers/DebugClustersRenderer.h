#pragma once

#include "../vulkan/VulkanRenderer.h"
#include "../vulkan/resource/ResourceTypes.h"
#include "../vulkan/resource/VulkanResourceManager.h"
#include "../scene/BatchManager.h"
#include "../vulkan/resource/ResourceFlags.h"
#include "../../debug/SprLog.h"
#include "../scene/Material.h"
#include "scene/SceneData.h"
#include "vulkan/gfx_vulkan_core.h"

namespace spr::gfx {
class DebugClustersRenderer {
public:
    DebugClustersRenderer(){}
    DebugClustersRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
    }
    ~DebugClustersRenderer(){}


    void init(
        Handle<DescriptorSet> globalDescSet, 
        Handle<DescriptorSetLayout> globalDescSetLayout,
        Handle<DescriptorSet> frameDescSets,
        Handle<DescriptorSetLayout> frameDescSetLayout,
        Handle<TextureAttachment> depthAttachment,
        Handle<TextureAttachment> visibilityTexture,
        Handle<TextureAttachment> shadowCascades[MAX_CASCADES],
        Handle<Buffer> shadowData,
        Handle<TextureAttachment> volumetricLighting,
        Handle<DescriptorSetLayout> descriptorSetLayout,
        Handle<DescriptorSet> descriptorSet,
        Handle<DescriptorSet> lightClusterDescSet,
        Handle<DescriptorSetLayout> lightClusterDescSetLayout)
    {
        m_globalDescSet = globalDescSet;
        m_globalDescSetLayout = globalDescSetLayout;
        m_frameDescSets = frameDescSets;
        m_frameDescSetLayout = frameDescSetLayout;
        m_descriptorSetLayout = descriptorSetLayout;
        m_descriptorSet = descriptorSet;
        m_lightClusterDescSet = lightClusterDescSet;
        m_lightClusterDescSetLayout = lightClusterDescSetLayout;

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
                .texture = depthAttachment,
                .loadOp = Flags::LoadOp::LOAD,
                .layout = Flags::ImageLayout::READ_ONLY,
                .finalLayout = Flags::ImageLayout::READ_ONLY
            },
            .colorAttachments = {
                {
                    .texture = m_attachment,
                    .finalLayout = Flags::ImageLayout::READ_ONLY
                }
            }
        });
        
        // shader
        m_shader = m_rm->create<Shader>({
            .vertexShader   = {.path = "../data/shaders/spv/debug_clusters.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/debug_clusters.frag.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { frameDescSetLayout },
                { m_descriptorSetLayout },
                { lightClusterDescSetLayout }
            },
            .graphicsState = {
                .depthTest = Flags::Compare::GREATER_OR_EQUAL,
                .depthWriteEnabled = false,
                .renderPass = m_renderPass,
            }
        });
    }


    void render(CommandBuffer& cb, std::vector<Batch>& batches){
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(0.45098f,0.52549f,0.47058f,1.f));
        
        // std::vector<Batch> batches;
        // batchManager.getBatches({.hasAny = MTL_ALL}, batches);

        passRenderer.drawSubpass({
            .shader = m_shader, 
            .set0 =  m_globalDescSet,
            .set1 = m_frameDescSets, 
            .set2 = m_descriptorSet,
            .set3 = m_lightClusterDescSet},
            batches);

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

    void updateDescriptorSet(
            Handle<TextureAttachment> depthAttachment,
            Handle<TextureAttachment> visibilityTexture,
            Handle<TextureAttachment> shadowCascades[MAX_CASCADES],
            Handle<Buffer> shadowData,
            Handle<TextureAttachment> volumetricLighting){
        m_rm->remove<DescriptorSet>(m_descriptorSet);

        // descriptor set
        m_descriptorSet = m_rm->create<DescriptorSet>({
            .textures = {
                {
                    .attachment = depthAttachment,
                    .layout = Flags::ImageLayout::READ_ONLY
                },
                {
                    .attachment = visibilityTexture,
                    .layout = Flags::ImageLayout::READ_ONLY
                },
                {
                    .attachments = {shadowCascades, MAX_CASCADES},
                    .layout = Flags::ImageLayout::READ_ONLY
                },
                {
                    .attachment = volumetricLighting,
                    .layout = Flags::ImageLayout::READ_ONLY
                },
            },
            .buffers = {
                {
                    .dynamicBuffer = shadowData, 
                    .byteSize = (MAX_FRAME_COUNT * m_rm->alignedSize(sizeof(SunShadowData)))
                }
            },
            .layout = m_descriptorSetLayout
        });
    }


    void destroy(){
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        m_rm->remove<TextureAttachment>(m_attachment);
        SprLog::info("[DebugClustersRenderer] [destroy] destroyed...");
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

    Handle<DescriptorSet> m_globalDescSet;
    Handle<DescriptorSetLayout> m_globalDescSetLayout;
    Handle<DescriptorSet> m_frameDescSets;
    Handle<DescriptorSetLayout> m_frameDescSetLayout;
    Handle<DescriptorSetLayout> m_descriptorSetLayout;
    Handle<DescriptorSet> m_descriptorSet;
    Handle<DescriptorSet> m_lightClusterDescSet;
    Handle<DescriptorSetLayout> m_lightClusterDescSetLayout;

    friend class RenderCoordinator;
};
}