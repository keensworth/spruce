#pragma once

#include "glm/detail/qualifier.hpp"
#include "gfx_vulkan_core.h"
#include <string>
#include <vector>

namespace spr {
    class SprWindow;
}

namespace spr::gfx {

class VulkanDevice {
public:
    typedef enum QueueType {
        GRAPHICS,
        PRESENT,
        TRANSFER,
        COMPUTE
    } QueueType;

    VulkanDevice();
    ~VulkanDevice();

    void createInfo(SprWindow& window);
    void createInstance(SprWindow& window);
    void createPhysicalDevice();
    void createDevice(VkSurfaceKHR surface);
    void destroy();

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

    QueueFamilies getQueueFamilies(){
        return m_queueFamilyIndices;
    }

    VkQueue getQueue(QueueType queueType) {
        switch (queueType) {
            case GRAPHICS:
                return m_graphicsQueue;
                break;
            case PRESENT:
                return m_presentQueue;
                break;
            case TRANSFER:
                return m_transferQueue;
                break;
            case COMPUTE:
                return m_computeQueue;
                break;
            default:
                return m_graphicsQueue;
        }
    }


private: // owning
    VkDevice m_device;
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;

private: // non-owning
    VkPhysicalDevice m_physicalDevice;

    VkApplicationInfo m_appInfo;
    std::string m_appName;
    bool m_destroyed = false;

    std::vector<std::vector<float>> m_queuePriorities{
        {1.0f},
        {1.0f,1.0f},
        {1.0f,1.0f,1.0f},
        {1.0f,1.0f,1.0f,1.0f}
    };
    QueueFamilies m_queueFamilyIndices;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkQueue m_transferQueue;
    VkQueue m_computeQueue;

    uint32 m_instanceExtensionCount = 0;
    std::vector<const char*> m_instanceExtensionNames;

    uint32 m_deviceExtensionCount = 0;
    std::vector<const char*> m_deviceExtensionNames;

    uint32 m_validationLayerCount = 0;
    std::vector<const char*> m_validationLayers;

    uint32 m_deviceCount = 0;
    std::vector<VkPhysicalDevice> m_physicalDevices;

    void getExtensions(SprWindow& window);
    bool hasExtension(std::vector<const char*> extensions, const char* requested);
    bool hasLayer(std::vector<VkLayerProperties> layers, const char* requested);
    void enableValidationLayers(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
    void setupDebugMessenger();

    bool isSuperiorDevice(VkPhysicalDevice newPhysicalDevice, VkPhysicalDevice bestPhysicalDevice);
    bool hasGraphicsQueueFamily(VkPhysicalDevice device);
    std::vector<VkDeviceQueueCreateInfo> queryQueueFamilies(VkSurfaceKHR surface);
};
}