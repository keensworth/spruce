#include "VulkanRenderer.h"
#include "CommandPool.h"
#include "VulkanDevice.h"
#include "vulkan_core.h"
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"


namespace spr::gfx {

VulkanRenderer::VulkanRenderer() {

}

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

    // create frames // TODO
    for (uint32 frame = 0; frame < MAX_FRAME_COUNT; frame++){
        m_frames[frame] = {
            .frameId = frame
        };
    }

    // create command pools (1/frame/queue family)
    QueueFamilies queueFamilies = m_device.getQueueFamilies();
    for (uint32 frame = 0; frame < MAX_FRAME_COUNT; frame++){
        // graphics queue command pools
        m_commandPools[frame] = CommandPool(m_device, queueFamilies.graphicsFamilyIndex.value(), rm);

        // additional transfer queue command pools (if applicable)
        if (!queueFamilies.familyUnique(queueFamilies.transferFamilyIndex.value()))
            continue;
        m_transferCommandPools[frame] = CommandPool(m_device, queueFamilies.transferFamilyIndex.value(), rm);
    }

    // create allocator
    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .physicalDevice   = m_device.getPhysicalDevice(),
        .device           = m_device.getDevice(),
        .instance         = m_device.getInstance(),
        .vulkanApiVersion = VK_API_VERSION_1_2
    };
    vmaCreateAllocator(&allocatorCreateInfo, &m_allocator);
    
}

VulkanRenderer::~VulkanRenderer(){
}

VulkanDevice& VulkanRenderer::getDevice(){
    return m_device;
}

VmaAllocator& VulkanRenderer::getAllocator() {
    return m_allocator;
}

CommandBuffer& VulkanRenderer::beginCommandRecording(CommandType commandBufferType){
    // get the appropriate command buffer
    CommandBuffer& commandBuffer = (commandBufferType == CommandType::TRANSFER) ?
                                   m_transferCommandPools[m_currFrame].getCommandBuffer(CommandType::TRANSFER) :
                                   m_commandPools[m_currFrame].getCommandBuffer(commandBufferType);
                    
    // begin recording command buffer
    VkCommandBuffer vulkanCommandBuffer = commandBuffer.getCommandBuffer();
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    VK_CHECK(vkBeginCommandBuffer(vulkanCommandBuffer, &beginInfo));

    return commandBuffer;
}

RenderFrame& VulkanRenderer::beginFrame() {
    // reset command pools before use
    m_commandPools[m_currFrame % MAX_FRAME_COUNT].reset(m_currFrame);
    m_transferCommandPools[m_currFrame % MAX_FRAME_COUNT].reset(m_currFrame);
    
}

void VulkanRenderer::present(){
    //

    m_currFrame++;
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