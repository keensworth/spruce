#include "VulkanDisplay.h"
#include "vulkan_core.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx{

VulkanDisplay::VulkanDisplay(Window& window){
    m_window = window;
}

VulkanDisplay::~VulkanDisplay(){}

void VulkanDisplay::createSurface(VkInstance instance){
    if(!SDL_Vulkan_CreateSurface(m_window.getHandle(), instance, &m_surface))
        SprLog::fatal("VulkanDisplay: Failed to create surface");
}

void VulkanDisplay::createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, QueueFamilies families){
    // get capabilities, surface formats, and present modes
    querySwapChainSupport(physicalDevice);

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

    // check if present and graphics queue are the same
    bool graphicsPresentShared = families.graphicsFamilyIndex == families.presentFamilyIndex;
    uint32 queueFamilyIndices[] = {families.graphicsFamilyIndex.value(), families.presentFamilyIndex.value()};

    // create info
    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_surface,
        .minImageCount = m_imageCount,
        .imageFormat = m_format,
        .imageColorSpace = m_surfaceFormat.colorSpace,
        .imageExtent = m_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = graphicsPresentShared ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT,
        .queueFamilyIndexCount = graphicsPresentShared ? 0u : 2u,
        .pQueueFamilyIndices = graphicsPresentShared ? NULL : queueFamilyIndices,
        .preTransform = m_capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = m_presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };
    VK_CHECK(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapChain));

    // get images from swapchain
    vkGetSwapchainImagesKHR(device, m_swapChain, &m_imageCount, nullptr);
    m_images.resize(m_imageCount);
    vkGetSwapchainImagesKHR(device, m_swapChain, &m_imageCount, m_images.data());
}

void VulkanDisplay::createImageViews(VkDevice device){
    // resize image views
    m_imageViews.resize(m_images.size());

    // create image views for all images
    for (size_t i = 0; i < m_imageCount; i++) {
        VkImageViewCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };
        VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &m_imageViews[i]));
    }
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
        m_window.width(),
        m_window.height()
    };
    actualExtent.width = std::clamp(actualExtent.width, m_capabilities.minImageExtent.width, m_capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, m_capabilities.minImageExtent.height, m_capabilities.maxImageExtent.height);
    return actualExtent;
}

void VulkanDisplay::querySwapChainSupport(VkPhysicalDevice physicalDevice) {
    // get the physical device capabilities
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_surface, &m_capabilities);

    // gather all available surface formats
    uint32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, nullptr);

    if (formatCount == 0) 
        SprLog::fatal("Vulkan Display: No surface formats available");

    m_formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_surface, &formatCount, m_formats.data());
    
    // gather all present modes
    uint32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, nullptr);

    if (presentModeCount == 0)
        SprLog::fatal("Vulkan DIsplay: No present modes available");

    m_presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_surface, &presentModeCount, m_presentModes.data());
}

void VulkanDisplay::recreateSwapChain(){

}

}