#pragma once

#include "CommandBuffer.h"

namespace spr::gfx{

class CommandPool{
public:
    CommandPool(): m_transferCommandBuffer(CommandType::TRANSFER, VK_NULL_HANDLE, nullptr),
                          m_offscreenCommandBuffer(CommandType::OFFSCREEN, VK_NULL_HANDLE, nullptr),
                          m_mainCommandBuffer(CommandType::MAIN, VK_NULL_HANDLE, nullptr){}
    CommandPool(VkDevice device, uint32 familyIndex, VulkanResourceManager* rm);
    ~CommandPool();

    CommandBuffer& getCommandBuffer(CommandType commandType);
    void reset();

private:
    VkDevice m_device;
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;
    VulkanResourceManager* m_rm;

    CommandBuffer m_transferCommandBuffer;
    CommandBuffer m_offscreenCommandBuffer;
    CommandBuffer m_mainCommandBuffer;
};
}