#pragma once

#include <vulkan/vulkan_core.h>
#include "resource/ResourceTypes.h"
#include "../scene/Draw.h"
#include "resource/VulkanResourceManager.h"
#include "DescriptorSetHandler.h"

namespace spr::gfx{

typedef struct PassContext {
    Handle<Shader> shader;
    Handle<DescriptorSet> set0;
    Handle<DescriptorSet> set1;
    Handle<DescriptorSet> set2;
    Handle<DescriptorSet> set3;
} PassContext;


class RenderPassRenderer{
public:
    RenderPassRenderer();
    RenderPassRenderer(VulkanResourceManager* rm, VkCommandBuffer commandBuffer, glm::uvec3 dimensions, uint32 frameIndex);
    ~RenderPassRenderer();

    void drawSubpass(PassContext context, std::vector<Batch>& batches);

private:
    VulkanResourceManager* m_rm;
    VkCommandBuffer m_commandBuffer;
    DescriptorSetHandler m_descSetHandler;
    glm::uvec3 m_dim;
    uint32 m_frameIndex;
};
}