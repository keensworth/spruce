#pragma once

#include "vulkan_core.h"
#include "Window.h"
#include <vulkan/vulkan_core.h>
#include "../../core/util/FunctionStack.h"
#include "../../debug/SprLog.h"



namespace spr::gfx {
class VulkanDisplay{
public:
    VulkanDisplay();
    VulkanDisplay(Window* window);
    ~VulkanDisplay();

    void createSurface(VkInstance instance);
    uint32 createSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, QueueFamilies families);
    void createImageViews(VkDevice device);
    
    void cleanup(VkDevice device);
    void destroy(VkDevice device, VkInstance instance);

    VkSurfaceKHR getSurface();
    std::vector<VkImageView> getImageViews();
    VkFormat getSwapchainFormat();
    VkSwapchainKHR getSwapchain();
    

private:
    Window* m_window;
    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapchain;

    uint32 m_imageCount;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_frameBuffers;

    VkSurfaceFormatKHR m_surfaceFormat;
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_extent;
    VkFormat m_format;
    
    VkSurfaceCapabilitiesKHR m_capabilities;
    std::vector<VkSurfaceFormatKHR> m_formats;
    std::vector<VkPresentModeKHR> m_presentModes;

    bool m_imageViewsInitialized;
    bool m_swapchainInitialized;
    bool m_cleanedUp;
    bool m_destroyed;

    void querySwapchainSupport(VkPhysicalDevice physicalDevice);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat();
    VkPresentModeKHR chooseSwapPresentMode();
    VkExtent2D chooseSwapExtent();
};
}