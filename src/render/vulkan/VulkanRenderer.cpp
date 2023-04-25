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

VulkanRenderer::VulkanRenderer(Window* window) : m_display(window){
    // create device info, instance, and physical device
    m_device.createInfo(*window);
    SprLog::debug("[VulkanRenderer] create info");
    m_device.createInstance(*window);
    SprLog::debug("[VulkanRenderer] create instance");
    m_device.createPhysicalDevice();
    SprLog::debug("[VulkanRenderer] create physical device");

    SprLog::debug("[VulkanRenderer] vulkan display created");
    // create display surface
    m_display.createSurface(m_device.getInstance());
    SprLog::debug("[VulkanRenderer] display surface created");

    // create device with display surface
    m_device.createDevice(m_display.getSurface());
    SprLog::debug("[VulkanRenderer] device created");

    // create swapchain and respective image views
    m_imageCount = m_display.createSwapchain(m_device.getPhysicalDevice(), m_device.getDevice(), m_device.getQueueFamilies());
    SprLog::debug("[VulkanRenderer] display swapchain created");
    m_display.createImageViews(m_device.getDevice());
    SprLog::debug("[VulkanRenderer] display image views created");


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
    SprLog::debug("[VulkanRenderer] done.");
}

VulkanRenderer::~VulkanRenderer(){
    if (m_destroyed || !m_initialized)
        return;
    
    SprLog::warn("[VulkanRenderer] [~] Calling destroy() in destructor");
    destroy();    
}

void VulkanRenderer::init(VulkanResourceManager *rm){
    // create command pools (1/frame/queue family)
    QueueFamilies queueFamilies = m_device.getQueueFamilies();
    uint32 graphicsFamilyIndex = queueFamilies.graphicsFamilyIndex.has_value() ? queueFamilies.graphicsFamilyIndex.value() : 0;
    uint32 transferFamilyIndex = queueFamilies.transferFamilyIndex.has_value() ? queueFamilies.transferFamilyIndex.value() : 0;
    for (uint32 frameIndex = 0; frameIndex < MAX_FRAME_COUNT; frameIndex++){
        // graphics queue command pools
        m_commandPools[frameIndex].init(m_device, rm, graphicsFamilyIndex, frameIndex, m_frames[frameIndex]);
        SprLog::debug("[VulkanRenderer] created gfx command pool, frame: " + std::to_string(frameIndex));

        // additional transfer queue command pools (if applicable)
        SprLog::debug("[VulkanRenderer] ???");
        m_transferCommandPools[frameIndex].init(m_device, rm, transferFamilyIndex, frameIndex, m_frames[frameIndex]);
        SprLog::debug("[VulkanRenderer] created transfer command pool, frame: " + std::to_string(frameIndex));

        // setup semaphore dependencies between them
        CommandBuffer& offscreenCB = m_commandPools[frameIndex].getCommandBuffer(CommandType::OFFSCREEN);
        CommandBuffer& mainCB = m_commandPools[frameIndex].getCommandBuffer(CommandType::MAIN);
        CommandBuffer& transferCB = m_transferCommandPools[frameIndex].getCommandBuffer(CommandType::TRANSFER);

        // transfer wait/signal dependencies
        std::vector<VkSemaphore> transferWaitSem = {
            
        };
        std::vector<VkSemaphore> transferSignalSem = {
            offscreenCB.getSemaphore() 
        };
        
        // offscreen wait/signal dependencies
        std::vector<VkSemaphore> offscreenWaitSem = {
            offscreenCB.getSemaphore(), 
            
        };
        std::vector<VkSemaphore> offscreenSignalSem = {
            mainCB.getSemaphore() 
        };
        
        // main wait/signal dependencies
        std::vector<VkSemaphore> mainWaitSem = {
            mainCB.getSemaphore(),
            m_frames[frameIndex].acquiredSem 
        };
        std::vector<VkSemaphore> mainSignalSem = {
            m_frames[frameIndex].renderedSem
        };

        // set semaphore dependencies 
        transferCB.setSemaphoreDependencies(transferWaitSem, transferSignalSem);
        offscreenCB.setSemaphoreDependencies(offscreenWaitSem, offscreenSignalSem);
        mainCB.setSemaphoreDependencies(mainWaitSem, mainSignalSem);
        SprLog::debug("[VulkanRenderer] set semaphore dependencies");
    }

    // create upload handlers
    for (uint32 frameIndex = 0; frameIndex < MAX_FRAME_COUNT; frameIndex++){
        SprLog::debug("[VulkanRenderer] attempting to create upload handler...");
        CommandBuffer& transferCommandBuffer = m_transferCommandPools[frameIndex].getCommandBuffer(CommandType::TRANSFER);
        CommandBuffer& graphicsCommandBuffer = m_commandPools[frameIndex].getCommandBuffer(CommandType::OFFSCREEN);
        m_uploadHandlers[frameIndex].init(m_device, *rm, transferCommandBuffer, graphicsCommandBuffer);
        SprLog::debug("[VulkanRenderer] created upload handler, frame: " + std::to_string(frameIndex));
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
        m_commandPools[i].destroy();
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
    SprLog::debug("[VulkanRenderer] [beginFrame] Beginning frame: ", m_frameIndex);
    CommandPool& gfxCommandPool = m_commandPools[m_frameIndex];
    CommandPool& transferCommandPool = m_transferCommandPools[m_frameIndex];
    RenderFrame& renderFrame = m_frames[m_frameIndex];
    renderFrame.frameIndex = m_frameIndex;
    SprLog::debug("[VulkanRenderer] [beginFrame]     got pools+frame");

    // // wait for fences, so we can reset pools
    CommandBuffer& transferCB = transferCommandPool.getCommandBuffer(CommandType::TRANSFER);
    CommandBuffer& offscreenCB = gfxCommandPool.getCommandBuffer(CommandType::OFFSCREEN);
    CommandBuffer& mainCB = gfxCommandPool.getCommandBuffer(CommandType::MAIN);
    SprLog::debug("[VulkanRenderer] [beginFrame]     got cb");
    transferCB.waitFence();
    offscreenCB.waitFence();
    mainCB.waitFence();
    transferCB.resetFence();
    offscreenCB.resetFence();
    mainCB.resetFence();
    SprLog::debug("[VulkanRenderer] [beginFrame]     waited");
    
    // acquire swapchain image index
    VkResult result = vkAcquireNextImageKHR(m_device.getDevice(), m_display.getSwapchain(), UINT64_MAX, renderFrame.acquiredSem, renderFrame.acquiredFence, &(renderFrame.imageIndex));
    SprLog::debug("[VulkanRenderer] [beginFrame]     acquired sci");
    validateSwapchain(result, ACQUIRE);
    SprLog::debug("[VulkanRenderer] [beginFrame]     validated");

    // reset command pools before use
    // pass them current frame id
    gfxCommandPool.prepare(m_currFrameId);
    transferCommandPool.prepare(m_currFrameId);
    SprLog::debug("[VulkanRenderer] [beginFrame]     prepared c pools");

    // flush resources pending deletion
    rm->flushDeletionQueue(m_currFrameId);
    SprLog::debug("[VulkanRenderer] [beginFrame]     flushed rm del queue");
    
    return renderFrame;
}


void VulkanRenderer::present(RenderFrame& frame){
    // make sure swapchain images have been written to
    VkSemaphore waitSemaphore[] = {frame.renderedSem};
    VkSwapchainKHR swapchain[] = {m_display.getSwapchain()};

    SprLog::debug("[VulkanRenderer] [present] test: ", m_display.getSwapchain() == VK_NULL_HANDLE);
    SprLog::debug("[VulkanRenderer] [present] test: ", frame.renderedSem == VK_NULL_HANDLE);

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
    SprLog::debug("[VulkanRenderer] [present] presented ");
    validateSwapchain(result, PRESENT);
    SprLog::debug("[VulkanRenderer] [present] validated");

    m_currFrameId++;
}


CommandBuffer& VulkanRenderer::beginGraphicsCommands(CommandType commandBufferType){
    if (commandBufferType == TRANSFER) {
        SprLog::error("[VulkanRenderer] Not a graphics command buffer");
    }

    CommandBuffer& commandBuffer = m_commandPools[m_frameIndex].getCommandBuffer(commandBufferType);
    commandBuffer.begin();

    // need to make sure we have an image to write to
    if (commandBufferType == MAIN) {
        RenderFrame& renderFrame = m_frames[m_frameIndex];
        VK_CHECK(vkWaitForFences(m_device.getDevice(), 1, &(renderFrame.acquiredFence), VK_TRUE, UINT64_MAX));
        VK_CHECK(vkResetFences(m_device.getDevice(), 1, &(renderFrame.acquiredFence)));
    }
    
    // cleanup gfx barriers from upload
    if (commandBufferType == OFFSCREEN) {
        m_uploadHandlers[m_frameIndex].performGraphicsBarriers();
    }

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


void VulkanRenderer::wait(){
    vkDeviceWaitIdle(m_device.getDevice());
}

}