#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "Window.h"

namespace spr::gfx{
class VulkanRenderer{
public:
    VulkanRenderer(){}
    ~VulkanRenderer(){}


    void init();

    void insertMesh();
    void insertLight();
    void updateCamera();
    void render();

    void destroy();


private:
    VkApplicationInfo m_appInfo;
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_device;
    VkSurfaceKHR m_surface;
};
}