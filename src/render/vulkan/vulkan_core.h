#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>


#define VK_CHECK(call)                 \
	do {                               \
		VkResult result_ = call;       \
		assert(result_ == VK_SUCCESS); \
	} while (0)

