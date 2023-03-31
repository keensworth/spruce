#include "VulkanDevice.h"
#include "vulkan_core.h"
#include <vulkan/vulkan_core.h>

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
    
    SprLog::warn("[VulkanDevice] Validation layer: " + std::string(pCallbackData->pMessage));

    return VK_FALSE;
}


//  ██╗███╗  ██╗██╗████████╗
//  ██║████╗ ██║██║╚══██╔══╝
//  ██║██╔██╗██║██║   ██║   
//  ██║██║╚████║██║   ██║   
//  ██║██║ ╚███║██║   ██║   
//  ╚═╝╚═╝  ╚══╝╚═╝   ╚═╝   

VulkanDevice::VulkanDevice(){}

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
    m_destroyed = true;
}

void VulkanDevice::createInfo(Window& window){
    m_appName = std::string(window.title());
    VkApplicationInfo appInfo {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = m_appName.c_str(),
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "spruce",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2
    };

    m_appInfo = appInfo;
}

void VulkanDevice::createInstance(Window& window){
    // get extensions required by SDL
    getExtensions(window);

    // request validation layers if DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    enableValidationLayers(debugCreateInfo);
    
    // fill and create instance
    VkInstanceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = DEBUG ? (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo : NULL,
        .pApplicationInfo = &m_appInfo,
        .enabledLayerCount = m_validationLayerCount,
        .ppEnabledLayerNames = DEBUG ? (const char *const *)m_validationLayers.data() : NULL,
        .enabledExtensionCount = m_extensionCount,
        .ppEnabledExtensionNames = (const char *const *)m_extensionNames.data(),
    };
    VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));

    // enable debug messenger if DEBUG
    if (DEBUG)
        CreateDebugUtilsMessengerEXT(m_instance, &debugCreateInfo, nullptr, &m_debugMessenger);
}

void VulkanDevice::createPhysicalDevice(){
    // get available physical devices
    vkEnumeratePhysicalDevices(m_instance, &m_deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(m_deviceCount);
    vkEnumeratePhysicalDevices(m_instance, &m_deviceCount, m_physicalDevices.data());

    // iterete over devices, select the ideal device
    VkPhysicalDevice bestPhysicalDevice = VK_NULL_HANDLE;
    for (const auto& device : devices){
        if (isSuperiorDevice(device, bestPhysicalDevice))
            bestPhysicalDevice = device;
    }
}

void VulkanDevice::createDevice(VkSurfaceKHR surface){
    // get all queue families
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = queryQueueFamilies(surface);
    
    // fill info and create logical device
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo createInfo {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = DEBUG ? m_validationLayerCount : 0,
        .ppEnabledLayerNames = DEBUG ? (const char *const *)m_validationLayers.data() : NULL,
        .enabledExtensionCount = m_extensionCount,
        .ppEnabledExtensionNames = (const char *const *)m_extensionNames.data(),
        .pEnabledFeatures = &deviceFeatures,

    };
    VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));

    // get device queues
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamilyIndex.value(), m_queueFamilyIndices.graphicsQueueIndex.value(), &m_graphicsQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamilyIndex.value(), m_queueFamilyIndices.presentQueueIndex.value(), &m_presentQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.transferFamilyIndex.value(), m_queueFamilyIndices.transferQueueIndex.value(), &m_transferQueue);
    vkGetDeviceQueue(m_device, m_queueFamilyIndices.computeFamilyIndex.value(), m_queueFamilyIndices.computeQueueIndex.value(), &m_computeQueue);
}


//  ██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ 
//  ██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗
//  ███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝
//  ██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗
//  ██║  ██║███████╗███████╗██║     ███████╗██║  ██║
//  ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝

bool VulkanDevice::isSuperiorDevice(VkPhysicalDevice newPhysicalDevice, VkPhysicalDevice bestPhysicalDevice){
    // first device
    if (bestPhysicalDevice == VK_NULL_HANDLE)
        return true;

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
    uint32 newHeapSize = 0;
    for (const auto& heap : newDeviceHeaps){
        if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            newHeapSize = heap.size;
    }

    // determine max best device heap size
    uint32 bestHeapSize = 0;
    for (const auto& heap : bestDeviceHeaps){
        if (heap.flags & VkMemoryHeapFlagBits::VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            bestHeapSize = heap.size;
    }

    return (newHeapSize > bestHeapSize);
}

bool VulkanDevice::hasGraphicsQueueFamily(VkPhysicalDevice device){
    // get queue families in device
    uint32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
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
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilies.data());

    // check for transfer only
    int transferIndex = 0;
    for (const auto& queueFamily : queueFamilies) {
        if ((queueFamily.queueFlags & !VK_QUEUE_GRAPHICS_BIT) &&
            (queueFamily.queueFlags & !VK_QUEUE_COMPUTE_BIT ) &&
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
        if ((queueFamily.queueFlags & !VK_QUEUE_GRAPHICS_BIT) &&
            (queueFamily.queueFlags &  VK_QUEUE_COMPUTE_BIT)){
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
        // check for graphics
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            m_queueFamilyIndices.graphicsFamilyIndex = familyIndex;
            m_queueFamilyIndices.graphicsQueueIndex = queueIndex;
            queueIndex++;
        }
        // check for present
        VkBool32 presentSupported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, familyIndex, surface, &presentSupported);
        if (presentSupported){
            m_queueFamilyIndices.graphicsFamilyIndex = familyIndex;
            m_queueFamilyIndices.graphicsQueueIndex = queueIndex;
            queueIndex++;
        }
        
        familyIndex++;
    }

    // get device queue createinfo
    float queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (int familyIndex = 0; familyIndex < queueFamilyCount; familyIndex++){
        uint32 queuesInFamily = m_queueFamilyIndices.getUniqueQueueCount(familyIndex);
        if (!queuesInFamily)
            continue;

        // create queue info for this family
        // might be combined queues, or separate
        queueCreateInfos.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = (uint32)familyIndex,
            .queueCount = queuesInFamily,
            .pQueuePriorities = &queuePriority
        });
    }

    return queueCreateInfos;
}

void VulkanDevice::getExtensions(Window& window){
    // query SDL for required extensions
    SDL_Vulkan_GetInstanceExtensions(window.getHandle(), &m_extensionCount, (const char **)m_extensionNames.data());

    // add swapchain extension
    bool requestedExtensionsFound = false;
    for (std::string extension : m_extensionNames){
        if (extension.compare(VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0){
            requestedExtensionsFound = true;
            break;
        }
    }
    if (!requestedExtensionsFound)
        m_extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // add additional extensions
    m_extensionNames.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);

    // add debug extension (if applicable)
    if (!DEBUG)
        return;

    bool debugUtilsFound = false;
    for (std::string extension : m_extensionNames){
        if (extension.compare("VK_EXT_debug_utils") == 0){
            debugUtilsFound = true;
            break;
        }
    }
    if (!debugUtilsFound)
        m_extensionNames.push_back("VK_EXT_debug_utils");
}

void VulkanDevice::enableValidationLayers(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo){
    // enable validation layers if in debug
    if (!DEBUG)
        return;

    // standard validation layer
    m_validationLayers.push_back("VK_LAYER_KHRONOS_validation");

    // get validation layers
    vkEnumerateInstanceLayerProperties(&m_validationLayerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(m_validationLayerCount);
    vkEnumerateInstanceLayerProperties(&m_validationLayerCount, availableLayers.data());

    // match requested validation layers with found
    for (std::string layerName : m_validationLayers) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (layerName.compare(layerProperties.layerName) == 0){
                layerFound = true;
                break;
            }
        }

        if (!layerFound){
            // validation layer not found
            SprLog::warn("[VulkanDevice] Validation layer not found: " + layerName);
            m_validationLayerCount = 0;
            m_validationLayers.clear();
            return;
        }
        m_validationLayerCount++;
    }

    // setup debug messenger
    debugCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback
    };
    
}
}