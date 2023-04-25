#include "VulkanDisplay.h"
#include "SDL_stdinc.h"
#include "VulkanDevice.h"
#include "../../external/volk/volk.h"
#include "../../debug/SprLog.h"
#include "../../interface/Window.h"
#include "SDL_vulkan.h"
#include <algorithm>

namespace spr::gfx{


//  ██╗███╗  ██╗██╗████████╗
//  ██║████╗ ██║██║╚══██╔══╝
//  ██║██╔██╗██║██║   ██║   
//  ██║██║╚████║██║   ██║   
//  ██║██║ ╚███║██║   ██║   
//  ╚═╝╚═╝  ╚══╝╚═╝   ╚═╝   

VulkanDisplay::VulkanDisplay(){
    
}

VulkanDisplay::VulkanDisplay(Window* window){
    m_window = window;
}

VulkanDisplay::~VulkanDisplay(){
    if (!m_destroyed)
        SprLog::error("[VulkanDisplay] [~] 'destroy' must be called before destructing - Improper release of resources");
}

void VulkanDisplay::createSurface(VkInstance instance){
    if(SDL_Vulkan_CreateSurface(m_window->getHandle(), instance, &m_surface) != SDL_TRUE)
        SprLog::fatal("[VulkanDisplay] Failed to create surface");
}

uint32 VulkanDisplay::createSwapchain(VkPhysicalDevice physicalDevice, VkDevice device, QueueFamilies families){
    // get capabilities, surface formats, and present modes
    querySwapchainSupport(physicalDevice);

    // choose appropriate from available
    m_surfaceFormat = chooseSwapSurfaceFormat();
    m_presentMode = chooseSwapPresentMode();
    m_extent = chooseSwapExtent();
    m_format = m_surfaceFormat.format;

    // get image count
    m_imageCount = m_capabilities.minImageCount + 1;
    if (m_capabilities.maxImageCount > 0 && m_imageCount > m_capabilities.maxImageCount) {
        m_imageCount = m_capabilities.maxImageCount;
    }

    // // check if present and graphics queue are the same
    bool graphicsPresentShared = true;
    // uint32 queueFamilyIndices[2];// = {families.graphicsFamilyIndex.value(), families.presentFamilyIndex.value()};
    // if (families.graphicsFamilyIndex.has_value() && families.presentFamilyIndex.has_value())
    //     queueFamilyIndices = {families.graphicsFamilyIndex.value(), families.presentFamilyIndex.value()};

    // create info
    VkSwapchainCreateInfoKHR createInfo = {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext            = NULL,
        .flags            = 0,
        .surface          = m_surface,
        .minImageCount    = m_imageCount,
        .imageFormat      = m_format,
        .imageColorSpace  = m_surfaceFormat.colorSpace,
        .imageExtent      = m_extent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = graphicsPresentShared ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = graphicsPresentShared ? 0u : 2u,
        .pQueueFamilyIndices   = NULL,//graphicsPresentShared ? NULL : queueFamilyIndices,
        .preTransform    = m_capabilities.currentTransform,
        .compositeAlpha  = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode     = m_presentMode,
        .clipped         = VK_TRUE,
        .oldSwapchain    = VK_NULL_HANDLE,
    };
    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain));

    // get images from swapchain
    vkGetSwapchainImagesKHR(device, m_swapchain, &m_imageCount, nullptr);
    m_images.resize(m_imageCount);
    vkGetSwapchainImagesKHR(device, m_swapchain, &m_imageCount, m_images.data());

    m_swapchainInitialized = true;
    return m_imageCount;
}

void VulkanDisplay::createImageViews(VkDevice device){
    assert(m_swapchainInitialized);

    // resize image views
    m_imageViews.resize(m_images.size());

    // create image views for all images
    for (uint32 i = 0; i < m_imageCount; i++) {
        VkImageViewCreateInfo createInfo{
            .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext      = NULL,
            .flags      = 0,
            .image      = m_images[i],
            .viewType   = VK_IMAGE_VIEW_TYPE_2D,
            .format     = m_format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        };
        VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &m_imageViews[i]));
    }

    m_imageViewsInitialized = true;
    m_cleanedUp = false;
}



//  ██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ 
//  ██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗
//  ███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝
//  ██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗
//  ██║  ██║███████╗███████╗██║     ███████╗██║  ██║
//  ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝

void VulkanDisplay::cleanup(VkDevice device){
    for (auto imageView : m_imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, m_swapchain, nullptr);

    m_imageViewsInitialized = false;
    m_swapchainInitialized = false;
    m_cleanedUp = true;
}

void VulkanDisplay::destroy(VkDevice device, VkInstance instance){
    if (!m_cleanedUp)
        cleanup(device);
    
    vkDestroySurfaceKHR(instance, m_surface, nullptr);

    m_destroyed = true;
    SprLog::info("[VulkanDisplay] [destroy] destroyed...");
}

VkSwapchainKHR VulkanDisplay::getSwapchain(){
    if (!m_swapchainInitialized)
        SprLog::error("[VulkanDisplay] Failed to retrieve swapchain, unitialized");

    return m_swapchain;
}

std::vector<VkImageView>& VulkanDisplay::getImageViews(){
    if (!m_imageViewsInitialized)
        SprLog::error("[VulkanDisplay] Failed to retrieve image views, unitialized");

    return m_imageViews;
}

VkFormat VulkanDisplay::getSwapchainFormat(){
    return m_format;
}


VkSurfaceKHR VulkanDisplay::getSurface(){
    return m_surface;
}

VkSurfaceFormatKHR VulkanDisplay::chooseSwapSurfaceFormat() {
    // query swapchain surface formats for desired
    for (const auto& format : m_formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    // return first
    return m_formats[0];
}

VkPresentModeKHR VulkanDisplay::chooseSwapPresentMode() {
    // query present modes for mailbox
    for (const auto& presentMode : m_presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }
    // return gauranteed fifo
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanDisplay::chooseSwapExtent() {
    // return current extent if valid
    if (m_capabilities.currentExtent.width != std::numeric_limits<uint32>::max()) {
        return m_capabilities.currentExtent;
    }

    // get actual extent
    VkExtent2D actualExtent = {
        m_window->width(),
        m_window->height()
    };
    actualExtent.width = std::clamp(actualExtent.width, m_capabilities.minImageExtent.width, m_capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, m_capabilities.minImageExtent.height, m_capabilities.maxImageExtent.height);
    return actualExtent;
}

void VulkanDisplay::querySwapchainSupport(VkPhysicalDevice physicalDevice) {
    // get the physical device capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &m_capabilities);

    // gather all available surface formats
    uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, nullptr);

    if (formatCount == 0) 
        SprLog::fatal("[VulkanDisplay] No surface formats available");

    m_formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, m_formats.data());
    
    // gather all present modes
    uint32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, nullptr);

    if (presentModeCount == 0)
        SprLog::fatal("[VulkanDisplay] No present modes available");

    m_presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, m_presentModes.data());
}

}