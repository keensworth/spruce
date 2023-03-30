#pragma once

#include "VulkanDevice.h"
#include "VulkanDisplay.h"
#include "CommandPool.h"
#include "RenderFrame.h"
#include "Window.h"
#include "vulkan_core.h"
#include "UploadHandler.h"

namespace spr::gfx{

class VulkanRenderer{
public:
    VulkanRenderer();
    VulkanRenderer(Window* window);
    ~VulkanRenderer();

    void init(VulkanResourceManager* rm);

    RenderFrame& beginFrame();
    void present(RenderFrame& frame);
    CommandBuffer& beginGraphicsCommands(CommandType commandBufferType);
    UploadHandler& beginTransferCommands();

    void onResize();
    void wait();
    
    uint32 getFrameId();
    VulkanDevice& getDevice();
    VulkanDisplay& getDisplay();

private:
    typedef enum SwapchainStage : uint32 {
        ACQUIRE = 0,
        PRESENT = 1
    } SwapchainStage;

    void recreateSwapchain();
    void validateSwapchain(VkResult result, SwapchainStage stage);
    void cleanup();

private:
    VulkanDevice m_device;
    VulkanDisplay m_display;

    CommandPool m_commandPools[MAX_FRAME_COUNT];
    CommandPool m_transferCommandPools[MAX_FRAME_COUNT];

    UploadHandler m_uploadHandlers[MAX_FRAME_COUNT];
    RenderFrame m_frames[MAX_FRAME_COUNT];

    uint32 m_imageCount = 0;
    uint32 m_currFrameId = 0;
    uint32 m_frameIndex = 0;
};
}