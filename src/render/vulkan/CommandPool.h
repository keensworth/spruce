#pragma once

#include "CommandBuffer.h"
#include "../../external/volk/volk.h"

namespace spr::gfx{

class VulkanDevice;
class VulkanResourceManager;
class RenderFrame;

class CommandPool{
public:
    CommandPool();
    ~CommandPool();

    CommandBuffer& getCommandBuffer(CommandType commandType);
    void prepare(uint32 frameId);

    void init(VulkanDevice& device, VulkanResourceManager* rm, uint32 familyIndex, uint32 frameIndex, RenderFrame& frame);
    void destroy();

private: // owning
    VkCommandPool m_commandPool;
    std::vector<VkCommandBuffer> m_commandBuffers;

private: // non-owning
    VulkanDevice* m_device;
    VulkanResourceManager* m_rm;

    CommandBuffer m_transferCommandBuffer;
    CommandBuffer m_offscreenCommandBuffer;
    CommandBuffer m_mainCommandBuffer;

    uint32 m_frameId;
    uint32 m_frameIndex;

    bool m_initialized = false;
    bool m_destroyed = false;
};
}