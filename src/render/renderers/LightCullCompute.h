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

struct Cluster {
    uint offset;
    uint count;
};

class LightCullCompute {
public:
    LightCullCompute(){}
    LightCullCompute(VulkanResourceManager& rm, VulkanRenderer& renderer){
        m_rm = &rm;
        m_renderer = &renderer;
        for (uint32 i = 0; i < MAX_FRAME_COUNT; i++)
            m_shadowTemp[i]= TempBuffer<SunShadowData>(1);
    }
    ~LightCullCompute(){}

    void init(
        Handle<DescriptorSet> globalDescSet, 
        Handle<DescriptorSetLayout> globalDescSetLayout,
        Handle<DescriptorSet> frameDescSets,
        Handle<DescriptorSetLayout> frameDescSetLayout)
    {
        m_globalDescSet = globalDescSet;
        m_globalDescSetLayout = globalDescSetLayout;
        m_frameDescSets = frameDescSets;
        m_frameDescSetLayout = frameDescSetLayout;

        // cluster buffers
        m_clusterListBuffer = m_rm->create<Buffer>({
            .byteSize = MAX_FRAME_COUNT * m_rm->alignedSize(16*8*24 * sizeof(Cluster)),
            .usage = Flags::BU_STORAGE_BUFFER,
            .memType = DEVICE
        });
        m_lightListBuffer = m_rm->create<Buffer>({
            .byteSize = MAX_FRAME_COUNT * MAX_LIGHTS * 256 * sizeof(uint32),
            .usage = Flags::BU_STORAGE_BUFFER,
            .memType = DEVICE
        });
        m_globalIndexCount = m_rm->create<Buffer>({
            .byteSize = MAX_FRAME_COUNT * m_rm->alignedSize(sizeof(uint32)),
            .usage = Flags::BU_STORAGE_BUFFER,
            .memType = DEVICE
        });

        // descriptor set layout
        m_descSetLayout = m_rm->create<DescriptorSetLayout>({
            .buffers = {
                {.binding = 0, .type = Flags::STORAGE_BUFFER}, // cluster list
                {.binding = 1, .type = Flags::STORAGE_BUFFER}, // light list
                {.binding = 2, .type = Flags::STORAGE_BUFFER}, // global index count
            }
        });

        // shader
        m_shader = m_rm->create<Shader>({
            .computeShader  = {.path = "../data/shaders/spv/light_cull.comp.spv"},
            .descriptorSets = {
                { globalDescSetLayout },
                { frameDescSetLayout },
                { m_descSetLayout },
                { }  // unused
            }
        });

        // descriptor set
        m_descSet = m_rm->create<DescriptorSet>({
            .buffers = {
                {
                    .dynamicBuffer = m_clusterListBuffer, 
                    .byteSize = (MAX_FRAME_COUNT * m_rm->alignedSize(16*8*24 * sizeof(Cluster)))
                },
                {
                    .dynamicBuffer = m_lightListBuffer, 
                    .byteSize = (MAX_FRAME_COUNT * MAX_LIGHTS * 256 * sizeof(uint32))
                },
                {
                    .dynamicBuffer = m_globalIndexCount, 
                    .byteSize = (MAX_FRAME_COUNT * m_rm->alignedSize(sizeof(uint32)))
                },
            },
            .layout = m_descSetLayout
        });
    }


    void dispatch(CommandBuffer& cb){
        RenderPassRenderer& passRenderer = cb.beginComputePass();
        passRenderer.dispatch({
            .shader = m_shader, 
            .set0 =  m_globalDescSet,
            .set1 = m_frameDescSets,
            .set2 = m_descSet},
            {1, 1, 24}
        );
        cb.endComputePass();
    }


    Handle<DescriptorSet> getDescSet(){
        return m_descSet;
    }

    Handle<DescriptorSetLayout> getDescSetLayout(){
        return m_descSetLayout;
    }

    Handle<Shader> getShader(){
        return m_shader;
    }


    void destroy(){
        m_rm->remove<DescriptorSet>(m_descSet);
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<DescriptorSetLayout>(m_descSetLayout);
        m_rm->remove<Buffer>(m_clusterListBuffer);
        m_rm->remove<Buffer>(m_lightListBuffer);
        m_rm->remove<Buffer>(m_globalIndexCount);

        for (uint32 i = 0; i < MAX_FRAME_COUNT; i++)
            m_shadowTemp[i].destroy();
        
        SprLog::info("[LightCullCompute] [destroy] destroyed...");
    }

private: // owning
    Handle<Shader> m_shader;
    Handle<DescriptorSet> m_descSet;
    Handle<DescriptorSetLayout> m_descSetLayout;
    Handle<Buffer> m_clusterListBuffer;
    Handle<Buffer> m_lightListBuffer;
    Handle<Buffer> m_globalIndexCount;

    TempBuffer<SunShadowData> m_shadowTemp[MAX_FRAME_COUNT];

private: // non-owning
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;

    Handle<DescriptorSet> m_globalDescSet;
    Handle<DescriptorSetLayout> m_globalDescSetLayout;
    Handle<DescriptorSet> m_frameDescSets;
    Handle<DescriptorSetLayout> m_frameDescSetLayout;
};
}