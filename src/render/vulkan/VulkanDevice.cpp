#include "VulkanDevice.h"
#include "vulkan_core.h"

namespace spr::gfx {

    VulkanDevice::VulkanDevice(Window& window){
        createInfo(window);
        createInstance(window);
        pickPhysicalDevice();
        createDevice();
    }

    VulkanDevice::~VulkanDevice(){
        for (int i = 0; i < m_extensionCount; i++){
            const char* extensionName = m_extensionNames[i];
            free((char*)extensionName);
        }
        free(m_extensionNames);

        vkDestroyInstance(m_instance, nullptr);
    }


    void VulkanDevice::createInfo(Window& window){
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = window.title().c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "spruce";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;
    }

    void VulkanDevice::createInstance(Window& window){
        SDL_Vulkan_GetInstanceExtensions(window.getHandle(), &m_extensionCount, (const char **)m_extensionNames);
        VkInstanceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = NULL,
            .pApplicationInfo = &m_appInfo,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = NULL,
            .enabledExtensionCount = m_extensionCount,
            .ppEnabledExtensionNames = (const char *const *)m_extensionNames,
        };

        VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
    }

    void VulkanDevice::pickPhysicalDevice(){

    }

    void VulkanDevice::createDevice(){

    }

}