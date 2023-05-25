#include "VulkanRenderer.h"
#include "CommandBuffer.h"
#include "CommandPool.h"
#include "VulkanDevice.h"
#include "resource/VulkanResourceManager.h"
#include <cstdint>
#include <string>
#include "../../external/volk/volk.h"
#include "../../debug/SprLog.h"

namespace spr::gfx {

VulkanRenderer::VulkanRenderer() {

}

VulkanRenderer::VulkanRenderer(SprWindow* window) : m_display(window){
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
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0
        };
        VK_CHECK(vkCreateSemaphore(m_device.getDevice(), &semaphoreInfo, NULL, &m_frames[frameIndex].acquiredSem));
        VK_CHECK(vkCreateSemaphore(m_device.getDevice(), &semaphoreInfo, NULL, &m_frames[frameIndex].renderedSem));

        // build fence info and create fence
        VkFenceCreateInfo fenceInfo {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = NULL,
        };
        VK_CHECK(vkCreateFence(m_device.getDevice(), &fenceInfo, NULL, &m_frames[frameIndex].acquiredFence));
    }
}

VulkanRenderer::~VulkanRenderer(){
    if (m_destroyed || !m_initialized)
        return;
    
    SprLog::warn("[VulkanRenderer] [~] Calling destroy() in destructor");
    destroy();    
}

void VulkanRenderer::init(VulkanResourceManager *rm){
    QueueFamilies queueFamilies = m_device.getQueueFamilies();
    uint32 graphicsFamilyIndex = queueFamilies.graphicsFamilyIndex.has_value() ? queueFamilies.graphicsFamilyIndex.value() : 0;
    uint32 transferFamilyIndex = queueFamilies.transferFamilyIndex.has_value() ? queueFamilies.transferFamilyIndex.value() : 0;

    // create command pools (1 for each queue family, per frame)
    for (uint32 frameIndex = 0; frameIndex < MAX_FRAME_COUNT; frameIndex++){
        // graphics queue command pools
        m_gfxCommandPools[frameIndex].init(m_device, rm, graphicsFamilyIndex, frameIndex, m_frames[frameIndex]);

        // additional transfer queue command pools (if applicable)
        m_transferCommandPools[frameIndex].init(m_device, rm, transferFamilyIndex, frameIndex, m_frames[frameIndex]);

        // setup semaphore dependencies between them
        CommandBuffer& offscreenCB = m_gfxCommandPools[frameIndex].getCommandBuffer(CommandType::OFFSCREEN);
        CommandBuffer& mainCB = m_gfxCommandPools[frameIndex].getCommandBuffer(CommandType::MAIN);
        CommandBuffer& transferCB = m_transferCommandPools[frameIndex].getCommandBuffer(CommandType::TRANSFER);

        // set semaphore dependencies ({wait}, {signal})
        transferCB.setSemaphoreDependencies(
            { },
            { offscreenCB.getSemaphore() }
        );
        offscreenCB.setSemaphoreDependencies(
            { offscreenCB.getSemaphore() },
            { mainCB.getSemaphore() }
        );
        mainCB.setSemaphoreDependencies(
            { mainCB.getSemaphore(), m_frames[frameIndex].acquiredSem },
            { m_frames[frameIndex].renderedSem}
        );
    }

    // create upload handlers
    for (uint32 frameIndex = 0; frameIndex < MAX_FRAME_COUNT; frameIndex++){
        CommandBuffer& transferCommandBuffer = m_transferCommandPools[frameIndex].getCommandBuffer(CommandType::TRANSFER);
        CommandBuffer& graphicsCommandBuffer = m_gfxCommandPools[frameIndex].getCommandBuffer(CommandType::OFFSCREEN);
        m_uploadHandlers[frameIndex].init(m_device, *rm, transferCommandBuffer, graphicsCommandBuffer);
    }

    m_initialized = true;
}

void VulkanRenderer::cleanup(){
    // destroy upload handlers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_uploadHandlers[i].destroy();
    }

    // command pools
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_gfxCommandPools[i].destroy();
        m_transferCommandPools[i].destroy();
    }

    m_display.cleanup(m_device.getDevice());
}

void VulkanRenderer::destroy(){
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
    SprLog::info("[VulkanRenderer] [destroy] destroyed...");
}


RenderFrame& VulkanRenderer::beginFrame(VulkanResourceManager* rm){
    m_frameIndex = m_currFrameId % MAX_FRAME_COUNT;
    CommandPool& gfxCommandPool = m_gfxCommandPools[m_frameIndex];
    CommandPool& transferCommandPool = m_transferCommandPools[m_frameIndex];
    RenderFrame& renderFrame = m_frames[m_frameIndex];
    renderFrame.frameIndex = m_frameIndex;

    // wait for fences, so we can reset pools
    CommandBuffer& transferCB = transferCommandPool.getCommandBuffer(CommandType::TRANSFER);
    CommandBuffer& offscreenCB = gfxCommandPool.getCommandBuffer(CommandType::OFFSCREEN);
    CommandBuffer& mainCB = gfxCommandPool.getCommandBuffer(CommandType::MAIN);
    transferCB.waitFence();
    offscreenCB.waitFence();
    mainCB.waitFence();
    transferCB.resetFence();
    offscreenCB.resetFence();
    mainCB.resetFence();
    
    // acquire swapchain image index
    VkResult result = vkAcquireNextImageKHR(
                            m_device.getDevice(), 
                            m_display.getSwapchain(), 
                            UINT64_MAX, 
                            renderFrame.acquiredSem, 
                            renderFrame.acquiredFence, 
                            &(renderFrame.imageIndex));
    validateSwapchain(result, ACQUIRE);

    // reset command pools before use
    // pass them current frame id
    gfxCommandPool.prepare(m_currFrameId);
    transferCommandPool.prepare(m_currFrameId);

    // flush resources pending deletion
    rm->flushDeletionQueue(m_currFrameId);
    
    return renderFrame;
}


void VulkanRenderer::present(RenderFrame& frame){
    // make sure swapchain images have been written to
    VkSemaphore waitSemaphore[] = {frame.renderedSem};
    VkSwapchainKHR swapchain[] = {m_display.getSwapchain()};


    // create present info and present frame
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = NULL,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphore,
        .swapchainCount = 1,
        .pSwapchains = swapchain,
        .pImageIndices = &frame.imageIndex,
        .pResults = NULL
    };
    VkQueue presentQueue = m_device.getQueue(VulkanDevice::GRAPHICS);
    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
    validateSwapchain(result, PRESENT);

    m_currFrameId++;
}


CommandBuffer& VulkanRenderer::beginGraphicsCommands(CommandType commandType){
    if (commandType == TRANSFER)
        SprLog::error("[VulkanRenderer] [beginGraphicsCommands] Not a graphics command buffer");

    CommandBuffer& commandBuffer = m_gfxCommandPools[m_frameIndex].getCommandBuffer(commandType);
    commandBuffer.begin();

    // need to make sure we have a swapchain image to write to
    if (commandType == MAIN) {
        RenderFrame& renderFrame = m_frames[m_frameIndex];
        VK_CHECK(vkWaitForFences(m_device.getDevice(), 1, &(renderFrame.acquiredFence), VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(m_device.getDevice(), 1, &(renderFrame.acquiredFence)));
    }
    
    // need to execute gfx barriers from any potential
    // transfer queue uploads
    if (commandType == OFFSCREEN)
        m_uploadHandlers[m_frameIndex].performGraphicsBarriers();

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
        SprLog::error("[VulkanRenderer] [validateSwapchain] Swapchain invalid, code: ", (uint32)result);
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


void VulkanRenderer::wait(){
    vkDeviceWaitIdle(m_device.getDevice());
}

}