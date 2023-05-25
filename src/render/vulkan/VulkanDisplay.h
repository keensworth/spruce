#pragma once

#include "gfx_vulkan_core.h"
#include "../../external/volk/volk.h"
#include <vector>

namespace spr {
    class SprWindow;
}

namespace spr::gfx {
class VulkanDisplay{
public:
    VulkanDisplay();
    VulkanDisplay(SprWindow* window);
    ~VulkanDisplay();

    void createSurface(VkInstance instance);
    uint32 createSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, QueueFamilies families);
    void createImageViews(VkDevice device);
    
    void cleanup(VkDevice device);
    void destroy(VkDevice device, VkInstance instance);

    VkSurfaceKHR getSurface();
    std::vector<VkImageView>& getImageViews();
    VkFormat getSwapchainFormat();
    VkSwapchainKHR getSwapchain();
    

private: //owning    
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;
    std::vector<VkImageView> m_imageViews;

private: // non-owning
    SprWindow* m_window;

    uint32 m_imageCount;
    std::vector<VkImage> m_images;

    VkSurfaceFormatKHR m_surfaceFormat;
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_extent;
    VkFormat m_format;
    
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    std::vector<VkPresentModeKHR> m_presentModes;

    bool m_imageViewsInitialized = false;
    bool m_swapchainInitialized = false;
    bool m_cleanedUp = true;
    bool m_destroyed = false;

    void querySwapchainSupport(VkPhysicalDevice physicalDevice);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat();
    VkPresentModeKHR chooseSwapPresentMode();
    VkExtent2D chooseSwapExtent();
};
}