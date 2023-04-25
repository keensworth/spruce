#include "CommandPool.h"
#include "CommandBuffer.h"
#include "../../external/volk/volk.h"
#include "RenderFrame.h"
#include "VulkanDevice.h"
#include "../../debug/SprLog.h"


namespace spr::gfx {

CommandPool::CommandPool(){}

CommandPool::~CommandPool(){
    if (m_destroyed || !m_initialized)
        return;
    
    SprLog::warn("[CommandPool] [~] Calling destroy() in destructor");
    destroy();
}

void CommandPool::init(VulkanDevice& device, VulkanResourceManager* rm, uint32 familyIndex, uint32 frameIndex, RenderFrame &frame){
    m_device = &device;
    m_rm = rm;
    m_frameId = 0;
    m_frameIndex = frameIndex;
    
    // build command pool info and create command pool
    VkCommandPoolCreateInfo commandPoolInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext            = NULL,
        .flags            = 0,
        .queueFamilyIndex = familyIndex
    };
    VK_CHECK(vkCreateCommandPool(m_device->getDevice(), &commandPoolInfo, NULL, &m_commandPool));

    // allocate command buffers
    m_commandBuffers = std::vector<VkCommandBuffer>(3);
    VkCommandBufferAllocateInfo cbAllocInfo {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = m_commandPool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32)m_commandBuffers.size()
    };
    VK_CHECK(vkAllocateCommandBuffers(m_device->getDevice(), &cbAllocInfo, m_commandBuffers.data()));

    // distribute command buffers
    m_transferCommandBuffer.init(device, rm, frameIndex, CommandType::TRANSFER, m_commandBuffers[0], m_device->getQueue(VulkanDevice::QueueType::TRANSFER));
    m_offscreenCommandBuffer.init(device, rm, frameIndex, CommandType::OFFSCREEN, m_commandBuffers[1], m_device->getQueue(VulkanDevice::QueueType::GRAPHICS));
    m_mainCommandBuffer.init(device, rm, frameIndex, CommandType::MAIN, m_commandBuffers[2], m_device->getQueue(VulkanDevice::QueueType::GRAPHICS));
    
    m_initialized = true;
}

void CommandPool::destroy(){
    // teardown command buffers
    m_transferCommandBuffer.destroy();
    m_offscreenCommandBuffer.destroy();
    m_mainCommandBuffer.destroy();

    // teardown command pool (and VkCommandBuffers with it)
    vkDestroyCommandPool(m_device->getDevice(), m_commandPool, nullptr);
    m_destroyed = true;
    SprLog::info("[CommandPool] [destroy] destroyed...");
}

CommandBuffer& CommandPool::getCommandBuffer(CommandType commandType){
    if (commandType == CommandType::TRANSFER)
        return m_transferCommandBuffer;
    else if (commandType == CommandType::OFFSCREEN)
        return m_offscreenCommandBuffer;
    else if (commandType == CommandType::MAIN)
        return m_mainCommandBuffer;
    
    SprLog::error("[CommandPool] Unknown command buffer type");
    return m_mainCommandBuffer;    
}

void CommandPool::prepare(uint32 frameId){
    vkResetCommandPool(m_device->getDevice(), m_commandPool, 0);
    
    m_frameId = frameId;
    uint32 frameIndex = m_frameId % MAX_FRAME_COUNT;
    if (frameIndex != m_frameIndex){
        std::string errMsg("[CommandPool] Frame Index mismatch! ");
        std::string errInfo("Expected: " + std::to_string(m_frameIndex) + ", got: " + std::to_string(frameIndex));
        SprLog::error(errMsg + errInfo);
    }

    m_mainCommandBuffer.setFrameId(frameId);
    m_offscreenCommandBuffer.setFrameId(frameId);
    m_transferCommandBuffer.setFrameId(frameId);
}

}