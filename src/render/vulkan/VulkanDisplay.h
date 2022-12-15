#pragma once

#include "vulkan_core.h"
#include "Window.h"
#include <vulkan/vulkan_core.h>

#include "../../debug/SprLog.h"

namespace spr::gfx {

class VulkanDisplay{
public:
    VulkanDisplay(Window& window);
    ~VulkanDisplay();

    void createSurface(VkInstance instance);
    void createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, QueueFamilies families);
    void recreateSwapChain();
    void createImageViews(VkDevice device);


private:
    Window m_window;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapChain;

    uint32 m_imageCount;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;

    VkSurfaceFormatKHR m_surfaceFormat;
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_extent;
    VkFormat m_format;
    
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    std::vector<VkPresentModeKHR> m_presentModes;

    void querySwapChainSupport(VkPhysicalDevice physicalDevice);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat();
    VkPresentModeKHR chooseSwapPresentMode();
    VkExtent2D chooseSwapExtent();
};
}