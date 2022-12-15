#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "VulkanDevice.h"
#include "VulkanDisplay.h"
#include "Window.h"

namespace spr::gfx{
class VulkanRenderer{
public:
    VulkanRenderer();
    ~VulkanRenderer();


    void init();

    void insertMesh();
    void insertLight();
    void updateCamera();
    void render();

    void destroy();


private:
    VulkanDevice m_device;
    VulkanDisplay m_display;
};
}