#pragma once

#include "VulkanDevice.h"
#include "VulkanDisplay.h"
#include "CommandPool.h"
#include "RenderFrame.h"
#include "gfx_vulkan_core.h"
#include "UploadHandler.h"

namespace spr {
    class SprWindow;
}

namespace spr::gfx{

class VulkanRenderer{
public:
    VulkanRenderer();
    VulkanRenderer(SprWindow* window);
    ~VulkanRenderer();

    void init(VulkanResourceManager* rm);
    void cleanup();
    void destroy();

    RenderFrame& beginFrame(VulkanResourceManager* rm);
    void present(RenderFrame& frame);
    CommandBuffer& beginGraphicsCommands(CommandType commandBufferType);
    UploadHandler& beginTransferCommands();

    void onResize();
    void wait();
    bool invalidSwapchain(){
        return m_dirtySwapchain;
    }
    
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

private:
    VulkanDevice m_device;
    VulkanDisplay m_display;

    CommandPool m_gfxCommandPools[MAX_FRAME_COUNT];
    CommandPool m_transferCommandPools[MAX_FRAME_COUNT];

    UploadHandler m_uploadHandlers[MAX_FRAME_COUNT];
    RenderFrame m_frames[MAX_FRAME_COUNT];

    uint32 m_imageCount = 0;
    uint32 m_currFrameId = 0;
    uint32 m_frameIndex = 0;

    bool m_initialized = false;
    bool m_destroyed = false;
    bool m_dirtySwapchain = false;
    uint32 m_fenceDelay = 0;
};
}