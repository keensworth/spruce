#include "VulkanRenderer.h"
#include "vulkan_core.h"
#include <vulkan/vulkan_core.h>


namespace spr::gfx {

VulkanRenderer::VulkanRenderer(Window* window, VulkanResourceManager* rm) : m_device(VulkanDevice()), m_display(VulkanDisplay(window)){
    // create device info, instance, and physical device
    m_device.createInfo(*window);
    m_device.createInstance(*window);
    m_device.createPhysicalDevice();

    // create display surface
    m_display.createSurface(m_device.getInstance());

    // create device with display surface
    m_device.createDevice(m_display.getSurface());

    // create swapchain and respective image views
    m_imageCount = m_display.createSwapchain(m_device.getPhysicalDevice(), m_device.getDevice(), m_device.getQueueFamilies());
    m_display.createImageViews(m_device.getDevice());

    // create frames
    for (uint32 frame = 0; frame < MAX_FRAME_COUNT; frame++){
        m_frames[frame] = {
            .frameId = frame
        };
    }

    // create command pools (1/frame/queue family)
    QueueFamilies queueFamilies = m_device.getQueueFamilies();
    for (uint32 frame = 0; frame < MAX_FRAME_COUNT; frame++){
        // graphics queue command pools
        m_commandPools[frame] = CommandPool(m_device.getDevice(), queueFamilies.graphicsFamilyIndex.value(), rm);

        // additional transfer queue command pools (if applicable)
        if (!queueFamilies.familyUnique(queueFamilies.transferFamilyIndex.value()))
            continue;
        m_transferCommandPools[frame] = CommandPool(m_device.getDevice(), queueFamilies.transferFamilyIndex.value(), rm);
    }
}

VulkanRenderer::~VulkanRenderer(){
}

CommandBuffer& VulkanRenderer::beginCommandRecording(CommandType commandBufferType){
    if (commandBufferType == CommandType::TRANSFER) {
        // get command buffer from transfer pool
        CommandBuffer& commandBuffer = m_transferCommandPools[m_currFrame].getCommandBuffer(CommandType::TRANSFER);

        // begin recording command buffer
        VkCommandBuffer vulkanCommandBuffer = commandBuffer.getCommandBuffer();
        VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };
        VK_CHECK(vkBeginCommandBuffer(vulkanCommandBuffer, &beginInfo));

        return commandBuffer;
    } 
    else {
        // get command buffer from transfer pool
        CommandBuffer& commandBuffer = m_commandPools[m_currFrame].getCommandBuffer(commandBufferType);

        // begin recording command buffer
        VkCommandBuffer vulkanCommandBuffer = commandBuffer.getCommandBuffer();
        VkCommandBufferBeginInfo beginInfo {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };
        VK_CHECK(vkBeginCommandBuffer(vulkanCommandBuffer, &beginInfo));

        return commandBuffer;
    }
    
}

RenderFrame& VulkanRenderer::beginFrame(){

}

void VulkanRenderer::present(){

}

void VulkanRenderer::wait(){
    vkDeviceWaitIdle(m_device.getDevice());
}

VkFormat VulkanRenderer::getDisplayFormat(){
    return m_display.getSwapchainFormat();
}

void VulkanRenderer::onResize(){
    // cleanup display
    m_display.cleanup(m_device.getDevice());
}

void VulkanRenderer::recreateSwapchain(){
    // create swapchain and image views
    m_display.createSwapchain(m_device.getPhysicalDevice(), m_device.getDevice(), m_device.getQueueFamilies());
    m_display.createImageViews(m_device.getDevice());
}

void VulkanRenderer::cleanup(){
    // cleanup display
    m_display.cleanup(m_device.getDevice());
}

}