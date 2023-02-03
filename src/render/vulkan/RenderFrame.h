#pragma once

#include "vulkan_core.h"

namespace spr::gfx{

typedef struct RenderFrame {
    VkFence fence = VK_NULL_HANDLE;
    VkSemaphore acquireSemaphore = VK_NULL_HANDLE;
    VkSemaphore presentSem = VK_NULL_HANDLE;
    uint32 frameId = 0;
} RenderFrame;

};
