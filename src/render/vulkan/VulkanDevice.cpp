#include "VulkanDevice.h"

#include "../../external/volk/volk.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "../../debug/SprLog.h"
#include "../../interface/Window.h"
#include "SDL_vulkan.h"
#include "gfx_vulkan_core.h"
#include <cstring>

namespace spr::gfx {

//   █████╗  █████╗ ██╗     ██╗     ██████╗  █████╗  █████╗ ██╗  ██╗
//  ██╔══██╗██╔══██╗██║     ██║     ██╔══██╗██╔══██╗██╔══██╗██║ ██╔╝
//  ██║  ╚═╝███████║██║     ██║     ██████╦╝███████║██║  ╚═╝█████═╝ 
//  ██║  ██╗██╔══██║██║     ██║     ██╔══██╗██╔══██║██║  ██╗██╔═██╗ 
//  ╚█████╔╝██║  ██║███████╗███████╗██████╦╝██║  ██║╚█████╔╝██║ ╚██╗
//   ╚════╝ ╚═╝  ╚═╝╚══════╝╚══════╝╚═════╝ ╚═╝  ╚═╝ ╚════╝ ╚═╝  ╚═╝

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {
    
    SprLog::info("[VK VALIDATION LAYERS]: " + std::string(pCallbackData->pMessage));

    return VK_FALSE;
}


//  ██╗███╗  ██╗██╗████████╗
//  ██║████╗ ██║██║╚══██╔══╝
//  ██║██╔██╗██║██║   ██║   
//  ██║██║╚████║██║   ██║   
//  ██║██║ ╚███║██║   ██║   
//  ╚═╝╚═╝  ╚══╝╚═╝   ╚═╝   

VulkanDevice::VulkanDevice(){
    
}

VulkanDevice::~VulkanDevice(){
    if (m_destroyed)
        return;
    
    SprLog::warn("[VulkanDevice] [~] Calling destroy() in destructor");
    destroy();
}

void VulkanDevice::destroy(){
    vkDestroyDevice(m_device, nullptr);
    if (DEBUG)
        DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
    vkDestroyInstance(m_instance, nullptr);

    // for (uint32 i = 0; i < m_extensionCount; i++){
    //     free((void*)m_extensionNames[i]);
    // }

    // for (uint32 i = 0; i < m_validationLayerCount; i++){
    //     free((void*)m_validationLayers[i]);
    // }

    m_destroyed = true;
    SprLog::info("[VulkanDevice] [destroy] destroyed...");
}

void VulkanDevice::createInfo(Window& window){
    m_appName = std::string(window.title());
    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = m_appName.c_str(),
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "spruce",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_MAKE_API_VERSION(0, 1, 2, 170)
    };

    m_appInfo = appInfo;
}

void VulkanDevice::createInstance(Window& window){
    VK_CHECK(volkInitialize());
    // get extensions required by SDL
    getExtensions(window);

    // request validation layers if DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    enableValidationLayers(debugCreateInfo);
    
    // fill and create instance
    VkInstanceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = DEBUG ? (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo : NULL,
        .flags = 0,
        .pApplicationInfo = &m_appInfo,
        .enabledLayerCount = DEBUG ? m_validationLayerCount : 0,
        .ppEnabledLayerNames = DEBUG ? m_validationLayers.data() : NULL,
        .enabledExtensionCount = m_instanceExtensionCount,
        .ppEnabledExtensionNames = m_instanceExtensionNames.data(),
    };
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
    volkLoadInstanceOnly(m_instance);
    // enable debug messenger if DEBUG
    if (DEBUG)
        CreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger);
}

void VulkanDevice::createPhysicalDevice(){
    // get available physical devices
    vkEnumeratePhysicalDevices(m_instance, &m_deviceCount, nullptr);
    m_physicalDevices.resize(m_deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &m_deviceCount, m_physicalDevices.data());


    // iterete over devices, select the ideal device
    VkPhysicalDevice bestPhysicalDevice = m_physicalDevices[0];
    for (VkPhysicalDevice device : m_physicalDevices){
        if (isSuperiorDevice(device, bestPhysicalDevice)){
            bestPhysicalDevice = device;
        }
    }

    m_physicalDevice = bestPhysicalDevice;
    assert(m_physicalDevice != VK_NULL_HANDLE);
}

void VulkanDevice::createDevice(VkSurfaceKHR surface){
    // get all queue families
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = queryQueueFamilies(surface);
    
    // create physical device feature chain
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
        .pNext = NULL,
        .synchronization2 = true
    };

    VkPhysicalDeviceFeatures deviceFeatures = {
        .geometryShader = VK_TRUE
    };

    VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &sync2,
        .features = deviceFeatures
    };

    // create device
    VkDeviceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features2,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = DEBUG ? m_validationLayerCount : 0,
        .ppEnabledLayerNames = DEBUG ? m_validationLayers.data() : NULL,
        .enabledExtensionCount = m_deviceExtensionCount,
        .ppEnabledExtensionNames = m_deviceExtensionNames.data(),
        .pEnabledFeatures = NULL,

    };
    VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
    volkLoadDevice(m_device);

    // get device queues
    if (m_queueFamilyIndices.graphicsFamilyIndex.has_value() && m_queueFamilyIndices.graphicsQueueIndex.has_value()){
        vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamilyIndex.value(), m_queueFamilyIndices.graphicsQueueIndex.value(), &m_graphicsQueue);
    }
    if (m_queueFamilyIndices.presentFamilyIndex.has_value() && m_queueFamilyIndices.presentQueueIndex.has_value()){
        vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamilyIndex.value(), m_queueFamilyIndices.presentQueueIndex.value(), &m_presentQueue);
    }
    if (m_queueFamilyIndices.transferFamilyIndex.has_value() && m_queueFamilyIndices.transferQueueIndex.has_value()){
        vkGetDeviceQueue(m_device, m_queueFamilyIndices.transferFamilyIndex.value(), m_queueFamilyIndices.transferQueueIndex.value(), &m_transferQueue);
    }
    if (m_queueFamilyIndices.computeFamilyIndex.has_value() && m_queueFamilyIndices.computeQueueIndex.has_value()){
        vkGetDeviceQueue(m_device, m_queueFamilyIndices.computeFamilyIndex.value(), m_queueFamilyIndices.computeQueueIndex.value(), &m_computeQueue);
    }
    
}


//  ██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ 
//  ██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗
//  ███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝
//  ██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗
//  ██║  ██║███████╗███████╗██║     ███████╗██║  ██║
//  ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝

bool VulkanDevice::isSuperiorDevice(VkPhysicalDevice newPhysicalDevice, VkPhysicalDevice bestPhysicalDevice){
    // both discrete, determine best by memory
    // get device memory properties
    VkPhysicalDeviceMemoryProperties newMemoryProperties1{};  // new
    vkGetPhysicalDeviceMemoryProperties(newPhysicalDevice, &newMemoryProperties1);
    VkPhysicalDeviceMemoryProperties bestMemoryProperties1{}; // best
    vkGetPhysicalDeviceMemoryProperties(bestPhysicalDevice, &bestMemoryProperties1);

    // get heaps for both devices
    VkMemoryHeap* newHeapsPointer1 = newMemoryProperties1.memoryHeaps;   // new
    std::vector<VkMemoryHeap> newDeviceHeaps1(newHeapsPointer1, newHeapsPointer1 + newMemoryProperties1.memoryHeapCount);
    VkMemoryHeap* bestHeapsPointer1 = bestMemoryProperties1.memoryHeaps; // best
    std::vector<VkMemoryHeap> bestDeviceHeaps1(bestHeapsPointer1, bestHeapsPointer1 + bestMemoryProperties1.memoryHeapCount);

    // determine max new device heap size
    uint64 newHeapSize1 = 0;
    for (const auto& heap : newDeviceHeaps1){
        if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT && heap.size > newHeapSize1)
            newHeapSize1 = heap.size;
    }


    // determine max best device heap size
    uint64 bestHeapSize1 = 0;
    for (const auto& heap : bestDeviceHeaps1){
        if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT && heap.size > bestHeapSize1)
            bestHeapSize1 = heap.size;
    }
    // first device
    if (bestPhysicalDevice == VK_NULL_HANDLE){
        return true;
    }


    // get device properties
    VkPhysicalDeviceProperties newProperties{};  // new
    vkGetPhysicalDeviceProperties(newPhysicalDevice, &newProperties);
    VkPhysicalDeviceProperties bestProperties{}; // best
    vkGetPhysicalDeviceProperties(bestPhysicalDevice, &bestProperties);

    // get device types
    bool newIsDiscrete =  newProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    bool bestIsDiscrete = bestProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
    bool newIsIntegrated =  newProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    bool bestIsIntegrated = bestProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
    bool newIsOther =  !(newIsDiscrete || newIsIntegrated);
    bool bestIsOther = !(bestIsDiscrete || bestIsIntegrated);

    // both integrated, continue
    if (newIsIntegrated && bestIsIntegrated)
        return false;


    // new is superior
    if ((newIsDiscrete && bestIsIntegrated) || bestIsOther)
        return true;


    // new is inferior
    if ((bestIsDiscrete && newIsIntegrated) || newIsOther)
        return false;



    // check queue families
    bool newHasGraphics = hasGraphicsQueueFamily(newPhysicalDevice);
    bool bestHasGraphics = hasGraphicsQueueFamily(bestPhysicalDevice);
    
    if (!newHasGraphics)
        return false;

    
    if (!bestHasGraphics)
        return true;
    
    

    // both discrete, determine best by memory
    // get device memory properties
    VkPhysicalDeviceMemoryProperties newMemoryProperties{};  // new
    vkGetPhysicalDeviceMemoryProperties(newPhysicalDevice, &newMemoryProperties);
    VkPhysicalDeviceMemoryProperties bestMemoryProperties{}; // best
    vkGetPhysicalDeviceMemoryProperties(bestPhysicalDevice, &bestMemoryProperties);

    // get heaps for both devices
    VkMemoryHeap* newHeapsPointer = newMemoryProperties.memoryHeaps;   // new
    std::vector<VkMemoryHeap> newDeviceHeaps(newHeapsPointer, newHeapsPointer + newMemoryProperties.memoryHeapCount);
    VkMemoryHeap* bestHeapsPointer = bestMemoryProperties.memoryHeaps; // best
    std::vector<VkMemoryHeap> bestDeviceHeaps(bestHeapsPointer, bestHeapsPointer + bestMemoryProperties.memoryHeapCount);

    // determine max new device heap size
    uint64 newHeapSize = 0;
    for (const auto& heap : newDeviceHeaps){
        if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT && heap.size > newHeapSize)
            newHeapSize = heap.size;
    }


    // determine max best device heap size
    uint64 bestHeapSize = 0;
    for (const auto& heap : bestDeviceHeaps){
        if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT && heap.size > bestHeapSize)
            bestHeapSize = heap.size;
    }


    return (newHeapSize > bestHeapSize);
}

bool VulkanDevice::hasGraphicsQueueFamily(VkPhysicalDevice device){
    // get queue families in device
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    // check for graphics bit
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            return true;
    }

    return false;
}

std::vector<VkDeviceQueueCreateInfo> VulkanDevice::queryQueueFamilies(VkSurfaceKHR surface){
    // get queue families in device
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, NULL);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    // check for transfer only
    int transferIndex = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT ) &&
            (queueFamily.queueFlags &  VK_QUEUE_TRANSFER_BIT)){
            m_queueFamilyIndices.transferFamilyIndex = transferIndex;
            m_queueFamilyIndices.transferQueueIndex = 0;
            break;
        }
        transferIndex++;
    }

    // check for compute only
    int computeIndex = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (!(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            (queueFamily.queueFlags &  VK_QUEUE_COMPUTE_BIT) &&
            (computeIndex != transferIndex)){
            m_queueFamilyIndices.computeFamilyIndex = computeIndex;
            m_queueFamilyIndices.computeQueueIndex = 0;
            break;
        }
        computeIndex++;
    }

    // find remaining families, regardless of uniqueness
    uint32 familyIndex = 0;
    for (const auto& queueFamily : queueFamilies) {
        uint32 queueIndex = 0;

        // done building
        if (m_queueFamilyIndices.isComplete())
            break;
        // check for transfer
        if (!m_queueFamilyIndices.transferFamilyIndex.has_value() && (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)){
            m_queueFamilyIndices.transferFamilyIndex = familyIndex;
            m_queueFamilyIndices.transferQueueIndex = queueIndex;
            familyIndex++;
            continue;
        }
        // check for compute
        if (!m_queueFamilyIndices.computeFamilyIndex.has_value() && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)){
            m_queueFamilyIndices.computeFamilyIndex = familyIndex;
            m_queueFamilyIndices.computeQueueIndex = queueIndex;
            familyIndex++;
            continue;
        }
        // check for graphics + present
        VkBool32 presentSupported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, familyIndex, surface, &presentSupported);
        if (presentSupported && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)){
            m_queueFamilyIndices.graphicsFamilyIndex = familyIndex;
            m_queueFamilyIndices.graphicsQueueIndex = queueIndex;
            m_queueFamilyIndices.graphicsFamilyIndex = familyIndex;
            m_queueFamilyIndices.graphicsQueueIndex = queueIndex;
            queueIndex++;
        }
        
        familyIndex++;
    }

    // get device queue createinfo
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (int familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++){
        uint32 queuesInFamily = m_queueFamilyIndices.getUniqueQueueCount(familyIndex);
        if (!queuesInFamily)
            continue;

        // create queue info for this family
        // might be combined queues, or separate
        queueCreateInfos.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .queueFamilyIndex = (uint32)familyIndex,
            .queueCount = queuesInFamily,
            .pQueuePriorities = m_queuePriorities.at(queuesInFamily-1).data()
        });
    }

    return queueCreateInfos;
}

bool VulkanDevice::hasExtension(std::vector<const char*> extensions, const char* requested){
    for (const char* extension : extensions){
        if (std::strcmp(extension, requested) == 0){
            return true;
        }
    }
    return false;
}

void VulkanDevice::getExtensions(Window& window){
    m_instanceExtensionCount = 0;
    m_deviceExtensionCount = 0;

    // [instance]
    // query SDL for required extensions
    if (!SDL_Vulkan_GetInstanceExtensions(window.getHandle(), &m_instanceExtensionCount, NULL)) 
        SprLog::warn("[VulkanDevice] Failed to find count of SDL instance extensions");
    const char **extensionNames = static_cast<const char **>(std::malloc(sizeof(const char *) * m_instanceExtensionCount));
    if (!SDL_Vulkan_GetInstanceExtensions(window.getHandle(), &m_instanceExtensionCount, extensionNames)) 
        SprLog::warn("[VulkanDevice] Failed to find any SDL instance extensions");

    for (uint32 i = 0; i < m_instanceExtensionCount; i++){
        m_instanceExtensionNames.push_back(extensionNames[i]);
    }

    // [device]
    // add sync2 extension
    m_deviceExtensionNames.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
    m_deviceExtensionCount++;
    // add swapchain extension
    m_deviceExtensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    m_deviceExtensionCount++;
    
    // [instance]
    // add debug extension (if applicable)
    if (DEBUG){
        if (!hasExtension(m_instanceExtensionNames, "VK_EXT_debug_utils")){
            m_instanceExtensionNames.push_back("VK_EXT_debug_utils");
            m_instanceExtensionCount++;
        }
    }

    std::free(extensionNames);
}

bool VulkanDevice::hasLayer(std::vector<VkLayerProperties> layers, const char* requested){
    for (VkLayerProperties layer : layers){
        if (std::strcmp(layer.layerName, requested) == 0){
            return true;
        }
    }
    return false;
}

void VulkanDevice::enableValidationLayers(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo){
    // enable validation layers if in debug
    if (!DEBUG)
        return;

    // get validation layers
    uint32 availableLayersCount = 0;
    vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(availableLayersCount);
    vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers.data());

    m_validationLayerCount = 0;
    // get standard validation layer
    if (hasLayer(availableLayers, "VK_LAYER_KHRONOS_validation")){
        m_validationLayers.push_back("VK_LAYER_KHRONOS_validation");
        m_validationLayerCount++;
    }
    // VK_LAYER_LUNARG_api_dump
    // if (hasLayer(availableLayers, "VK_LAYER_LUNARG_api_dump")){
    //     m_validationLayers.push_back("VK_LAYER_LUNARG_api_dump");
    //     m_validationLayerCount++;
    // }
    // VK_LAYER_LUNARG_monitor
    if (hasLayer(availableLayers, "VK_LAYER_LUNARG_monitor")){
        m_validationLayers.push_back("VK_LAYER_LUNARG_monitor");
        m_validationLayerCount++;
    }
    // VK_LAYER_KHRONOS_synchronization2
    if (hasLayer(availableLayers, "VK_LAYER_KHRONOS_synchronization2")){
        m_validationLayers.push_back("VK_LAYER_KHRONOS_synchronization2");
        m_validationLayerCount++;
    }
    // setup debug messenger
    debugCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback
    };
    
}
}