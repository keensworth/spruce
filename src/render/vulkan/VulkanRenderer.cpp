#include "VulkanRenderer.h"
#include "CommandPool.h"
#include "VulkanDevice.h"
#include "vulkan_core.h"
#include <cstdint>
#include <string>
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

VulkanRenderer::VulkanRenderer() {

}

VulkanRenderer::VulkanRenderer(Window* window) : m_device(VulkanDevice()), m_display(VulkanDisplay(window)){
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
    for (uint32 frameIndex = 0; frameIndex < MAX_FRAME_COUNT; frameIndex++){
        m_frames[frameIndex] = {};

        // build semaphore info and create semaphore
        VkSemaphoreCreateInfo semaphoreInfo {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };
        VK_CHECK(vkCreateSemaphore(m_device.getDevice(), &semaphoreInfo, NULL, &m_frames[frameIndex].acquiredSem));
        VK_CHECK(vkCreateSemaphore(m_device.getDevice(), &semaphoreInfo, NULL, &m_frames[frameIndex].renderedSem));

        // build fence info and create fence
        VkFenceCreateInfo fenceInfo {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = NULL,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };
        VK_CHECK(vkCreateFence(m_device.getDevice(), &fenceInfo, NULL, &m_frames[frameIndex].acquiredFence));
    }
}

VulkanRenderer::~VulkanRenderer(){
    if (!m_destroyed)
        SprLog::error("[VulkanRenderer] [~] 'destroy' must be called before destructing - Improper release of resources");
}

void VulkanRenderer::init(VulkanResourceManager *rm){
    // create command pools (1/frame/queue family)
    QueueFamilies queueFamilies = m_device.getQueueFamilies();
    for (uint32 frameIndex = 0; frameIndex < MAX_FRAME_COUNT; frameIndex++){
        // graphics queue command pools
        m_commandPools[frameIndex] = CommandPool(m_device, queueFamilies.graphicsFamilyIndex.value(), rm, m_frames[frameIndex], frameIndex);

        // additional transfer queue command pools (if applicable)
        if (!queueFamilies.familyUnique(queueFamilies.transferFamilyIndex.value()))
            continue;
        m_transferCommandPools[frameIndex] = CommandPool(m_device, queueFamilies.transferFamilyIndex.value(), rm, m_frames[frameIndex], frameIndex);
    }

    // create upload handlers
    for (uint32 frameIndex = 0; frameIndex < MAX_FRAME_COUNT; frameIndex++){
        CommandBuffer& transferCommandBuffer = m_transferCommandPools[frameIndex].getCommandBuffer(CommandType::TRANSFER);
        CommandBuffer& graphicsCommandBuffer = m_commandPools[frameIndex].getCommandBuffer(CommandType::OFFSCREEN);
        m_uploadHandlers[frameIndex] = UploadHandler(m_device, *rm, transferCommandBuffer, graphicsCommandBuffer);
    }   
}

void VulkanRenderer::destroy(){
    // destroy upload handlers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_uploadHandlers[i].destroy();
    }

    // command pools
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_commandPools[i].destroy();
        m_transferCommandPools[i].destroy();
    }

    // frame sync structures
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        RenderFrame& renderFrame = m_frames[i];
        vkDestroySemaphore(m_device.getDevice(), renderFrame.renderedSem, nullptr);
        vkDestroySemaphore(m_device.getDevice(), renderFrame.acquiredSem, nullptr);
        vkDestroyFence(m_device.getDevice(), renderFrame.acquiredFence, nullptr);
    }

    // display + device
    m_display.destroy(m_device.getDevice(), m_device.getInstance());
    m_device.destroy();

    m_destroyed = true;
}


RenderFrame& VulkanRenderer::beginFrame(){
    m_frameIndex = m_currFrameId % MAX_FRAME_COUNT;

    CommandPool& gfxCommandPool = m_commandPools[m_frameIndex];
    CommandPool& transferCommandPool = m_transferCommandPools[m_frameIndex];
    RenderFrame& renderFrame = m_frames[m_frameIndex];
    renderFrame.frameIndex = m_frameIndex;

    // wait for main command buffer fence
    CommandBuffer& cb = gfxCommandPool.getCommandBuffer(CommandType::MAIN);
    cb.waitFence();
    
    // acquire swapchain image index
    VkResult result = vkAcquireNextImageKHR(m_device.getDevice(), m_display.getSwapchain(), UINT64_MAX, renderFrame.acquiredSem, VK_NULL_HANDLE, &(renderFrame.imageIndex));
    validateSwapchain(result, ACQUIRE);
    
    // reset command pools before use
    // pass them current frame id
    gfxCommandPool.prepare(m_currFrameId);
    transferCommandPool.prepare(m_currFrameId);
    
    return renderFrame;
}


void VulkanRenderer::present(RenderFrame& frame){
    VkSemaphore waitSemaphore[] = {frame.renderedSem};
    VkSwapchainKHR swapchain[] = {m_display.getSwapchain()};

    // create present info and present frame
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphore,
        .swapchainCount = 1,
        .pSwapchains = swapchain,
        .pImageIndices = &frame.imageIndex
    };
    VkQueue presentQueue = m_device.getQueue(VulkanDevice::PRESENT);
    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    validateSwapchain(result, PRESENT);

    m_currFrameId++;
}


CommandBuffer& VulkanRenderer::beginGraphicsCommands(CommandType commandBufferType){
    if (commandBufferType == TRANSFER) {
        SprLog::error("[VulkanRenderer] Not a graphics command buffer");
    }

    // get the appropriate command buffer and begin recording
    CommandBuffer& commandBuffer = m_commandPools[m_frameIndex].getCommandBuffer(commandBufferType);
    commandBuffer.begin();

    return commandBuffer;
}


UploadHandler& VulkanRenderer::beginTransferCommands(){
    // get the frame's upload handler
    UploadHandler& uploadHandler = m_uploadHandlers[m_frameIndex];
    uploadHandler.setFrameId(m_currFrameId);
    uploadHandler.reset();

    // get transfer command buffer and begin recording
    CommandBuffer& commandBuffer = m_transferCommandPools[m_frameIndex].getCommandBuffer(CommandType::TRANSFER);
    commandBuffer.begin();

    return uploadHandler;
}


uint32 VulkanRenderer::getFrameId(){
    return m_currFrameId;
}


VulkanDevice& VulkanRenderer::getDevice(){
    return m_device;
}


VulkanDisplay& VulkanRenderer::getDisplay(){
    return m_display;
}


void VulkanRenderer::validateSwapchain(VkResult result, SwapchainStage stage){
    if (result == VK_SUCCESS)
        return;
    
    // abnormal swapchain result, return error
    if (result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_SUBOPTIMAL_KHR){
        std::string message = "[VulkanRenderer] Swapchain invalid, code: ";
        message += std::to_string((uint32)result);
        SprLog::error(message);
        return;
    }

    // will need to recreate swapchain, as we know 
    // either OUT_OF_DATE or SUBOPTIMAL are true
    if (stage == PRESENT || (stage == ACQUIRE && result == VK_ERROR_OUT_OF_DATE_KHR)){
        recreateSwapchain();
    }
}


void VulkanRenderer::onResize(){
    wait();
    cleanup();
    recreateSwapchain();
}


void VulkanRenderer::recreateSwapchain(){
    m_display.createSwapchain(m_device.getPhysicalDevice(), m_device.getDevice(), m_device.getQueueFamilies());
    m_display.createImageViews(m_device.getDevice());
}


void VulkanRenderer::cleanup(){
    m_display.cleanup(m_device.getDevice());
}


void VulkanRenderer::wait(){
    vkDeviceWaitIdle(m_device.getDevice());
}

}