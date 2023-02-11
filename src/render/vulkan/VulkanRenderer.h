#pragma once

#include <vulkan/vulkan_core.h>
#include "VulkanDevice.h"
#include "VulkanDisplay.h"
#include "CommandPool.h"
#include "RenderFrame.h"
#include "Window.h"

namespace spr::gfx{

class VulkanRenderer{
public:
    VulkanRenderer();
    VulkanRenderer(Window* window, VulkanResourceManager* rm);
    ~VulkanRenderer();

    CommandBuffer& beginCommandRecording(CommandType commandBufferType);
    RenderFrame& beginFrame();
    void present();
    void onResize();
    void wait();
    
    VkFormat getDisplayFormat();
    VulkanDevice& getDevice();
    VmaAllocator& getAllocator();

private:
    
    uint32 m_framesInFlight = 2;
    uint32 m_imageCount = 0;
    uint32 m_currFrame = 0;

    RenderFrame m_frames[MAX_FRAME_COUNT];

    CommandPool m_commandPools[MAX_FRAME_COUNT];
    CommandPool m_transferCommandPools[MAX_FRAME_COUNT];

    VulkanDevice m_device;
    VulkanDisplay m_display;

    VmaAllocator m_allocator;

    void recreateSwapchain();
    void cleanup();
};
}