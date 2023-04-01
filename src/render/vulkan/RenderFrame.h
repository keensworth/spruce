#pragma once

#include "gfx_vulkan_core.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx{

typedef struct RenderFrame {
    VkFence acquiredFence     = VK_NULL_HANDLE;
    VkSemaphore acquiredSem   = VK_NULL_HANDLE;
    VkSemaphore renderedSem   = VK_NULL_HANDLE;
    uint32 frameIndex         = 0;
    uint32 imageIndex         = 0;
} RenderFrame;

};
