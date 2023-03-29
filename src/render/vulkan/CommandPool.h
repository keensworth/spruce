#pragma once

#include "CommandBuffer.h"
#include "VulkanDevice.h"
#include "RenderFrame.h"
#include "vulkan_core.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx{

class CommandPool{
public:
    CommandPool();
    CommandPool(VulkanDevice& device, uint32 familyIndex, VulkanResourceManager* rm, RenderFrame& frame, uint32 frameIndex);
    ~CommandPool();

    CommandBuffer& getCommandBuffer(CommandType commandType);
    void prepare(uint32 frameId);

private:
    VulkanDevice* m_device;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VulkanResourceManager* m_rm;

    uint32 m_frameId;
    uint32 m_frameIndex;

    CommandBuffer m_transferCommandBuffer;
    CommandBuffer m_offscreenCommandBuffer;
    CommandBuffer m_mainCommandBuffer;
};
}