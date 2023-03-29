#pragma once

#include "glm/detail/qualifier.hpp"
#include "vulkan_core.h"
#include "Window.h"
#include <vulkan/vulkan_core.h>
#include "../../debug/SprLog.h"
#include "../../core/util/FunctionStack.h"

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

    void createInfo(Window& window);
    void createInstance(Window& window);
    void createPhysicalDevice();
    void createDevice(VkSurfaceKHR surface);

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


private:
    VkApplicationInfo m_appInfo;
    std::string m_appName;
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;

    QueueFamilies m_queueFamilyIndices;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkQueue m_transferQueue;
    VkQueue m_computeQueue;

    VkDebugUtilsMessengerEXT m_debugMessenger;

    uint32 m_extensionCount = 0;
    std::vector<std::string> m_extensionNames;

    uint32 m_validationLayerCount = 0;
    std::vector<std::string> m_validationLayers;

    uint32 m_deviceCount = 0;
    std::vector<VkPhysicalDevice> m_physicalDevices;

    void getExtensions(Window& window);
    void enableValidationLayers(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo);
    void setupDebugMessenger();

    bool isSuperiorDevice(VkPhysicalDevice newPhysicalDevice, VkPhysicalDevice bestPhysicalDevice);
    bool hasGraphicsQueueFamily(VkPhysicalDevice device);
    std::vector<VkDeviceQueueCreateInfo> queryQueueFamilies(VkSurfaceKHR surface);
};
}