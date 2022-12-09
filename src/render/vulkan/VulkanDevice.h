#pragma once

#include "vulkan_core.h"
#include "Window.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {
class VulkanDevice{
public:
    VulkanDevice(Window& window);
    ~VulkanDevice();

    VkApplicationInfo& getInfo(){
        return m_appInfo;
    }

    VkInstance& getInstance(){
        return m_instance;
    }

    VkPhysicalDevice& getPhysicalDevice(){
        return m_physicalDevice;
    }

    VkDevice& getDevice(){
        return m_device;
    }


private:
    VkApplicationInfo m_appInfo;
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;

    uint32 m_extensionCount;
    char ** m_extensionNames;

    void createInfo(Window& window);
    void createInstance(Window& window);
    void pickPhysicalDevice();
    void createDevice();
};
}