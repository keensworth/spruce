#pragma once
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/gtc/matrix_inverse.hpp"
#include "vulkan/VulkanRenderer.h"
#include "vulkan/resource/ResourceTypes.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include "scene/BatchManager.h"
#include "vulkan/resource/ResourceFlags.h"
#include "debug/SprLog.h"
#include "scene/Material.h"
#include "scene/SceneData.h"
#include "vulkan/gfx_vulkan_core.h"
#include <glm/gtx/string_cast.hpp>

namespace spr::gfx {
class SunShadowRenderer {
public:
    SunShadowRenderer(){}
    SunShadowRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer){
        m_rm = &rm;
        m_renderer = &renderer;
        for (uint32 i = 0; i < MAX_FRAME_COUNT; i++)
            m_shadowTemp[i]= TempBuffer<SunShadowData>(1);
    }
    ~SunShadowRenderer(){}

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

        // shadow data buffer
        m_shadowBuffer = m_rm->create<Buffer>({
            .byteSize = (uint32) (MAX_FRAME_COUNT * m_rm->alignedSize(sizeof(SunShadowData))),
            .usage = Flags::BufferUsage::BU_UNIFORM_BUFFER |
                     Flags::BufferUsage::BU_TRANSFER_DST,
            .memType = DEVICE | HOST
        });

        // depth attachments
        for (uint32 i = 0; i < MAX_CASCADES; i++){
            m_cascadeDepths[i] = m_rm->create<TextureAttachment>({
                .textureLayout = {
                    .dimensions = {4096, 4096, 1},
                    .format = Flags::Format::D32_SFLOAT,
                    .usage = Flags::ImageUsage::IU_DEPTH_STENCIL_ATTACHMENT | 
                             Flags::ImageUsage::IU_SAMPLED,
                    .sampler = { .addressing = Flags::Wrap::CLAMP_TO_BORDER}
                }
            });
        }

        // render pass
        m_renderPassLayout = m_rm->create<RenderPassLayout>({
            .depthAttachmentFormat = Flags::D32_SFLOAT,
            .subpass = {
                .depthAttachment = 1,
            }
        });
        m_renderPass = m_rm->create<RenderPass>({
            .dimensions = {4096, 4096, 1},
            .layout = m_renderPassLayout,
            .depthAttachment = {
                .texture = m_cascadeDepths[0],
                .finalLayout = Flags::ImageLayout::READ_ONLY,
                .compareOp = Flags::Compare::GREATER_OR_EQUAL
            }
        });

        // framebuffers for remaining attachments
        // (renderpass builds first framebuffer)
        for (uint32 i = 0; i < MAX_CASCADES-1; i++){
            m_cascadeFramebuffers[i] = m_rm->create<Framebuffer>({
                .dimensions = {4096, 4096, 1},
                .renderPass = m_renderPass,
                .depthAttachment = {
                    .texture = m_cascadeDepths[i+1],
                    .finalLayout = Flags::ImageLayout::READ_ONLY,
                    .compareOp = Flags::Compare::GREATER_OR_EQUAL
                }
            });
        }

        // descriptor set layout
        m_descSetLayout = m_rm->create<DescriptorSetLayout>({
            .buffers = {
                {.binding = 0} // sun shadow data
            }
        });

        // shader
        m_shader = m_rm->create<Shader>({
            .vertexShader   = {.path = "../data/shaders/spv/shadow_cascades.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/shadow_cascades.frag.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { frameDescSetLayout },
                { m_descSetLayout },
                { }  // unused
            },
            .graphicsState = {
                .depthTest = Flags::Compare::GREATER_OR_EQUAL,
                .renderPass = m_renderPass,
            }
        });

        // descriptor set
        m_descSet = m_rm->create<DescriptorSet>({
            .buffers = {
                {
                    .dynamicBuffer = m_shadowBuffer, 
                    .byteSize = (MAX_FRAME_COUNT * m_rm->alignedSize(sizeof(SunShadowData)))
                }
            },
            .layout = m_descSetLayout
        });
    }


    void uploadData(Scene& scene, Camera& camera, Light& light, UploadHandler& uploadHandler, float splitLambda){
        uint32 frameId = m_renderer->getFrameId();
        m_shadowTemp[frameId % MAX_FRAME_COUNT].reset();
        m_shadowTemp[frameId % MAX_FRAME_COUNT].insert({});

        float cascadeSplitLambda = splitLambda;

        float cascadeSplits[MAX_CASCADES];

		float nearClip = camera.near;
		float farClip = camera.far;
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		// Calculate split depths based on view camera frustum
		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < MAX_CASCADES; i++) {
			float p = (i + 1) / static_cast<float>(MAX_CASCADES);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = cascadeSplitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearClip) / clipRange;
		}

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0f;
		for (uint32_t i = 0; i < MAX_CASCADES; i++) {
			float splitDist = cascadeSplits[i];

			glm::vec3 frustumCorners[8] = {
				glm::vec3(-1.0f,  1.0f, 1.0f),
				glm::vec3( 1.0f,  1.0f, 1.0f),
				glm::vec3( 1.0f, -1.0f, 1.0f),
				glm::vec3(-1.0f, -1.0f, 1.0f),
				glm::vec3(-1.0f,  1.0f,  0.0f),
				glm::vec3( 1.0f, -1.0f,  0.0f),
				glm::vec3( 1.0f,  1.0f,  0.0f),
				glm::vec3(-1.0f, -1.0f,  0.0f),
			};

			// Project frustum corners into world space
			glm::mat4 invCam = glm::inverse(scene.viewProj);
			for (uint32_t i = 0; i < 8; i++) {
				glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
				frustumCorners[i] = invCorner / invCorner.w;
			}
            
			for (uint32_t i = 0; i < 4; i++) {
				glm::vec3 dist = frustumCorners[i+4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}

			// Get frustum center
			glm::vec3 frustumCenter = glm::vec3(0.0f);
			for (uint32_t i = 0; i < 8; i++) {
				frustumCenter += frustumCorners[i];
			}
			frustumCenter /= 8.0f;

			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++) {
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

            glm::vec3 lightDir = glm::normalize(light.dir);  

            glm::vec3 eye = frustumCenter - lightDir * -minExtents.z;
            
			glm::mat4 lightViewMatrix = glm::lookAt(eye, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::mat4 lightOrthoMatrix = glm::orthoRH_ZO(minExtents.x, maxExtents.y, minExtents.x, maxExtents.y, maxExtents.z-minExtents.z, -8.f);

			// Store split distance and matrix in cascade
            SunShadowData& shadowData = m_shadowTemp[frameId % MAX_FRAME_COUNT][0];
			shadowData.cascadeSplit[0][i%4] = (nearClip + splitDist * clipRange); // world dist
            shadowData.cascadeSplit[1][i%4] = (splitDist); // normalized wrt clipRange
			shadowData.cascadeViewProj[i] = lightOrthoMatrix * lightViewMatrix;

			lastSplitDist = cascadeSplits[i];
		}

        // upload data
        uploadHandler.uploadDyanmicBuffer(m_shadowTemp[m_renderer->getFrameId() % MAX_FRAME_COUNT], m_shadowBuffer);
    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        Handle<DescriptorSet> currFrameDescSet = m_frameDescSets[m_renderer->getFrameId() % MAX_FRAME_COUNT];

        std::vector<Batch> batches;
        batchManager.getBatches({.hasAny = MTL_ALL}, batches);

        // render first shadow map (renderpass owns 1st framebuffer)
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass);
        passRenderer.drawSubpass({
            .shader = m_shader, 
            .set0 =  m_globalDescSet,
            .set1 = currFrameDescSet,
            .set2 = m_descSet}, 
            batches, 0
        );
        cb.endRenderPass();
        
        // render to remaining shadow maps (pass in additional framebuffers)
        for (uint32 i = 0; i < MAX_CASCADES-1; i++){
            passRenderer = cb.beginRenderPass(m_renderPass, m_cascadeFramebuffers[i]);
            passRenderer.drawSubpass({
                .shader = m_shader, 
                .set0 =  m_globalDescSet,
                .set1 = currFrameDescSet,
                .set2 = m_descSet}, 
                batches, i + 1
            );
            cb.endRenderPass();
        }
    }


    Handle<RenderPass> getRenderPass(){
        return m_renderPass;
    }

    Handle<TextureAttachment>* getDepthAttachments(){
        return m_cascadeDepths;
    }

    Handle<Buffer> getShadowData(){
        return m_shadowBuffer;
    }

    Handle<Shader> getShader(){
        return m_shader;
    }


    void destroy(){
        m_rm->remove<DescriptorSet>(m_descSet);
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<DescriptorSetLayout>(m_descSetLayout);
        for (Handle<Framebuffer> fb : m_cascadeFramebuffers)
            m_rm->remove<Framebuffer>(fb);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        for (Handle<TextureAttachment> att : m_cascadeDepths)
            m_rm->remove<TextureAttachment>(att);
        m_rm->remove<Buffer>(m_shadowBuffer);

        for (uint32 i = 0; i < MAX_FRAME_COUNT; i++)
            m_shadowTemp[i].reset();
        
        SprLog::info("[SunShadowRenderer] [destroy] destroyed...");
    }

private: // owning
    Handle<TextureAttachment> m_cascadeDepths[MAX_CASCADES];
    Handle<Framebuffer> m_cascadeFramebuffers[MAX_CASCADES-1];
    Handle<RenderPassLayout> m_renderPassLayout;
    Handle<RenderPass> m_renderPass;
    Handle<Shader> m_shader;
    Handle<DescriptorSet> m_descSet;
    Handle<DescriptorSetLayout> m_descSetLayout;
    Handle<Buffer> m_shadowBuffer;

    TempBuffer<SunShadowData> m_shadowTemp[MAX_FRAME_COUNT];

private: // non-owning
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;

    Handle<DescriptorSet> m_globalDescSet;
    Handle<DescriptorSetLayout> m_globalDescSetLayout;
    Handle<DescriptorSet> m_frameDescSets[MAX_FRAME_COUNT];
    Handle<DescriptorSetLayout> m_frameDescSetLayout;
};
}