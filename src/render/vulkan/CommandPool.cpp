#include "CommandPool.h"
#include "vulkan_core.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

CommandPool::CommandPool(){}

CommandPool::CommandPool(VulkanDevice device, uint32 familyIndex, VulkanResourceManager* rm){
    m_device = device;
    m_rm = rm;
    m_frame = 0;
    
    // build command pool create info
    VkCommandPoolCreateInfo commandPoolInfo = {
        .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags            = 0,
        .queueFamilyIndex = familyIndex
    };

    // create command pool
    VK_CHECK(vkCreateCommandPool(m_device.getDevice(), &commandPoolInfo, NULL, &m_commandPool));

    // allocate command buffers
    m_commandBuffers = std::vector<VkCommandBuffer>(3);
    VkCommandBufferAllocateInfo cbAllocInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = (uint32)m_commandBuffers.size()
    };
    VK_CHECK(vkAllocateCommandBuffers(m_device.getDevice(), &cbAllocInfo, m_commandBuffers.data()));

    // distribute command buffers
    m_transferCommandBuffer  = CommandBuffer(
                                            m_device, 
                                            CommandType::TRANSFER, 
                                            m_commandBuffers[0], 
                                            rm, 
                                            m_device.getQueue(VulkanDevice::QueueType::TRANSFER));
    m_offscreenCommandBuffer = CommandBuffer(
                                            m_device, 
                                            CommandType::OFFSCREEN, 
                                            m_commandBuffers[1], 
                                            rm, 
                                            m_device.getQueue(VulkanDevice::QueueType::GRAPHICS));
    m_mainCommandBuffer      = CommandBuffer(
                                            m_device, 
                                            CommandType::MAIN, 
                                            m_commandBuffers[2], 
                                            rm, 
                                            m_device.getQueue(VulkanDevice::QueueType::GRAPHICS));

    // set semaphore dependencies between command buffers
    //      transfer
    std::vector<VkSemaphore> transferWaitSem = {
        m_mainCommandBuffer.getSemaphore()
    };
    std::vector<VkSemaphore> transferSignalSem = {
        m_offscreenCommandBuffer.getSemaphore()
    };
    m_transferCommandBuffer.setSemaphoreDependencies(transferWaitSem, transferSignalSem);

    //      offscreen
    std::vector<VkSemaphore> offscreenWaitSem = {
        m_transferCommandBuffer.getSemaphore(),
        // TODO: present semaphore
    };
    std::vector<VkSemaphore> offscreenSignalSem = {
        m_mainCommandBuffer.getSemaphore()
    };
    m_offscreenCommandBuffer.setSemaphoreDependencies(offscreenWaitSem, offscreenSignalSem);

    //      main
    std::vector<VkSemaphore> mainWaitSem = {
        m_offscreenCommandBuffer.getSemaphore()
    };
    std::vector<VkSemaphore> mainSignalSem = {
        // TODO: present semaphore,
        m_transferCommandBuffer.getSemaphore()
    };
    m_mainCommandBuffer.setSemaphoreDependencies(mainWaitSem, mainSignalSem);

    //      present // TODO:
}

CommandPool::~CommandPool(){
    vkDestroyCommandPool(m_device.getDevice(), m_commandPool, NULL);
}

CommandBuffer& CommandPool::getCommandBuffer(CommandType commandType){
    if (commandType == CommandType::TRANSFER)
        return m_transferCommandBuffer;
    else if (commandType == CommandType::OFFSCREEN)
        return m_offscreenCommandBuffer;
    else if (commandType == CommandType::MAIN)
        return m_mainCommandBuffer;
    
    SprLog::error("CommandPool: Unknown command buffer type");
    return m_mainCommandBuffer;    
}

void CommandPool::reset(uint32 frame){
    vkResetCommandPool(m_device.getDevice(), m_commandPool, 0);
    
    m_frame = frame;

    m_mainCommandBuffer.setFrame(frame);
    m_offscreenCommandBuffer.setFrame(frame);
    m_transferCommandBuffer.setFrame(frame);
}

}