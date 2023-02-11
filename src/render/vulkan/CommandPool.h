#pragma once

#include "CommandBuffer.h"
#include "VulkanDevice.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx{

class CommandPool{
public:
    CommandPool();
    CommandPool(VulkanDevice device, uint32 familyIndex, VulkanResourceManager* rm);
    ~CommandPool();

    CommandBuffer& getCommandBuffer(CommandType commandType);
    void reset(uint32 frame);

private:
    VulkanDevice m_device;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VulkanResourceManager* m_rm;

    uint32 m_frame;

    CommandBuffer m_transferCommandBuffer;
    CommandBuffer m_offscreenCommandBuffer;
    CommandBuffer m_mainCommandBuffer;
};
}