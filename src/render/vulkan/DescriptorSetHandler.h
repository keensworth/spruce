#pragma once

#include "../../external/volk/volk.h"
#include "../core/memory/Handle.h"

namespace spr {
}

namespace spr::gfx{

class VulkanResourceManager;
struct DescriptorSet;

class DescriptorSetHandler{
public:
    DescriptorSetHandler();
    DescriptorSetHandler(VulkanResourceManager* rm, VkCommandBuffer commandBuffer);
    ~DescriptorSetHandler();

    void set(uint32 set, Handle<DescriptorSet> handle);
    void updateBindings(VkPipelineLayout pipelineLayout, uint32 frameIndex);
    void updateBindingsCompute(VkPipelineLayout pipelineLayout, uint32 frameIndex);
    void reset();

private: // non-owning
    VulkanResourceManager* m_rm;
    VkCommandBuffer m_commandBuffer;
    
    Handle<DescriptorSet> m_sets[4];

    bool m_initialized = false;
};
}