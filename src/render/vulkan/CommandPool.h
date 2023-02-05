#pragma once

#include "CommandBuffer.h"
#include "VulkanDevice.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx{

class CommandPool{
public:
    CommandPool(VulkanDevice device, uint32 familyIndex, VulkanResourceManager* rm);
    ~CommandPool();

    CommandBuffer& getCommandBuffer(CommandType commandType);
    void reset();

private:
    VulkanDevice m_device;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VulkanResourceManager* m_rm;

    CommandBuffer m_transferCommandBuffer;
    CommandBuffer m_offscreenCommandBuffer;
    CommandBuffer m_mainCommandBuffer;
};
}