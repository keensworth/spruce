#pragma once

#include "spruce_core.h"
#include <optional>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define DEBUG true

#define VK_CHECK(call)                 \
	do {                               \
		VkResult result_ = call;       \
		assert(result_ == VK_SUCCESS); \
	} while (0)

namespace spr::gfx {

static const uint32 MAX_FRAME_COUNT = 3;
static const uint32 MAX_DRAWS = 1<<17; // 2^18 draws


typedef struct QueueFamilies{
    std::optional<uint32> graphicsFamilyIndex;
    std::optional<uint32> graphicsQueueIndex;
    std::optional<uint32> presentFamilyIndex;
    std::optional<uint32> presentQueueIndex;
    std::optional<uint32> transferFamilyIndex;
    std::optional<uint32> transferQueueIndex;
    std::optional<uint32> computeFamilyIndex;
    std::optional<uint32> computeQueueIndex;

    bool isComplete() {
        return graphicsFamilyIndex.has_value() && graphicsQueueIndex.has_value() &&
               presentFamilyIndex.has_value()  && presentQueueIndex.has_value() &&
               transferFamilyIndex.has_value() && transferQueueIndex.has_value() &&
               computeFamilyIndex.has_value()  && computeQueueIndex.has_value();
    }

    bool familyUnique(uint32 familyIndex){
        uint32 queuesInFamily = 0;
        
        queuesInFamily += (graphicsFamilyIndex == familyIndex);
        queuesInFamily += (presentFamilyIndex == familyIndex);
        queuesInFamily += (transferFamilyIndex == familyIndex);
        queuesInFamily += (computeQueueIndex == familyIndex);

        return queuesInFamily == 1;
    }

    bool queueUnique(uint32 familyIndex, uint32 queueIndex){
        uint32 queueCount = 0;

        queueCount += (graphicsFamilyIndex == familyIndex) && (graphicsQueueIndex == queueIndex);
        queueCount += (presentFamilyIndex == familyIndex) && (presentQueueIndex == queueIndex);
        queueCount += (transferFamilyIndex == familyIndex) && (transferQueueIndex == queueIndex);
        queueCount += (computeQueueIndex == familyIndex) && (computeQueueIndex == queueIndex);

        return queueCount == 1;
    }

    uint32 getUniqueQueueCount(uint32 familyIndex){
        uint32 queuesInFamily = 0;
        // check if graphics in this family
        if (graphicsFamilyIndex == familyIndex){
            queuesInFamily++;
        }

        // check if present in this family
        if (presentFamilyIndex == familyIndex){
            queuesInFamily++;
        }

        // check if transfer in this family
        if (transferFamilyIndex == familyIndex){
            queuesInFamily++;
        }

        // check if compute in this family
        if (computeFamilyIndex == familyIndex){
            queuesInFamily++;
        }

        return queuesInFamily;
    }
} QueueFamilies;

}

