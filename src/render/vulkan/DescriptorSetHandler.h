#pragma once

#include <vulkan/vulkan_core.h>
#include "resource/ResourceTypes.h"
#include "../scene/Draw.h"
#include "resource/VulkanResourceManager.h"

namespace spr::gfx{

class DescriptorSetHandler{
public:
    DescriptorSetHandler();
    DescriptorSetHandler(VulkanResourceManager* rm, VkCommandBuffer commandBuffer);
    ~DescriptorSetHandler();

    void set(uint32 set, Handle<DescriptorSet> handle);
    void updateBindings(VkPipelineLayout pipelineLayout, uint32 frameIndex);
    void reset();

private: // non-owning
    VulkanResourceManager* m_rm;
    VkCommandBuffer m_commandBuffer;
    
    Handle<DescriptorSet> m_sets[4];
};
}