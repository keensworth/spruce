#pragma once

#include "../vulkan/VulkanRenderer.h"
#include "../vulkan/resource/ResourceTypes.h"
#include "../vulkan/resource/VulkanResourceManager.h"
#include "../scene/BatchManager.h"
#include "../vulkan/resource/ResourceFlags.h"
#include "../../debug/SprLog.h"
#include "../scene/Material.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {
class SkyboxRenderer {
public:
    SkyboxRenderer(){}
    SkyboxRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
    }
    ~SkyboxRenderer(){}


    void init(
        Handle<DescriptorSet> globalDescSet, 
        Handle<DescriptorSetLayout> globalDescSetLayout,
        Handle<DescriptorSet> frameDescSets,
        Handle<DescriptorSetLayout> frameDescSetLayout,
        Handle<DescriptorSet> litMeshDescSet,
        Handle<DescriptorSetLayout> litMeshDestSetLayout,
        Handle<TextureAttachment> depthAttachment,
        Handle<TextureAttachment> colorAttachment)
    {
        m_globalDescSet = globalDescSet;
        m_globalDescSetLayout = globalDescSetLayout;
        m_frameDescSets = frameDescSets;
        m_frameDescSetLayout = frameDescSetLayout;
        m_descriptorSetLayout = litMeshDestSetLayout;
        m_descriptorSet = litMeshDescSet;

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
                    .texture = colorAttachment,
                    .loadOp = Flags::LoadOp::LOAD,
                    .layout = Flags::ImageLayout::COLOR_ATTACHMENT,
                    .finalLayout = Flags::ImageLayout::READ_ONLY
                }
            }
        });

        // shader
        m_shader = m_rm->create<Shader>({
            .vertexShader   = {.path = "../data/shaders/spv/skybox.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/skybox.frag.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { frameDescSetLayout },
                { m_descriptorSetLayout }, 
                { }  // unused
            },
            .graphicsState = {
                .depthTest = Flags::Compare::LESS,
                .depthTestEnabled = false,
                .depthWriteEnabled = false,
                .cullMode = Flags::CullMode::FRONT,
                .renderPass = m_renderPass
                
            }
        });
    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(0.f, 0.f ,0.f ,1.f));
        // render unit cube around camera
        passRenderer.drawSubpass({
            .shader = m_shader,
            .set0 =  m_globalDescSet,
            .set1 = m_frameDescSets,
            .set2 = m_descriptorSet}, 
            batchManager.getCubeBatch(), batchManager.getCubeVertexOffset(), 0);

        cb.endRenderPass();
    }


    Handle<RenderPass> getRenderPass(){
        return m_renderPass;
    }

    Handle<Shader> getShader(){
        return m_shader;
    }

    void updateDescriptorSet(Handle<DescriptorSet> descriptorSet){
        m_descriptorSet = descriptorSet;
    }


    void destroy(){
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        SprLog::info("[SkyboxRenderer] [destroy] destroyed...");
    }

private: // owning
    
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
    Handle<DescriptorSet> m_descriptorSet;
    Handle<DescriptorSetLayout> m_descriptorSetLayout;
};
}