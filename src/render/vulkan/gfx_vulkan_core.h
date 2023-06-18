#pragma once

#include "spruce_core.h"
#include <optional>
#include "../../external/volk/volk.h"


#define SPR_DEBUG true

// #define VK_CHECK(call)                 \
// 	do {                               \
// 		VkResult result_ = call;       \
// 		assert(result_ == VK_SUCCESS); \
// 	} while (0)

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout << "VK_CHECK: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)

namespace spr::gfx {

static const uint32 MAX_FRAME_COUNT = 3;
static const uint32 MAX_DRAWS = 1<<18; // 2^18 draws


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
        
        if (graphicsFamilyIndex.has_value())
            queuesInFamily += (graphicsFamilyIndex.value() == familyIndex);
        if(presentFamilyIndex.has_value())
            queuesInFamily += (presentFamilyIndex.value() == familyIndex);
        if(transferFamilyIndex.has_value())
            queuesInFamily += (transferFamilyIndex.value() == familyIndex);
        if(computeQueueIndex.has_value())
            queuesInFamily += (computeQueueIndex.value() == familyIndex);

        return queuesInFamily == 1;
    }

    bool queueUnique(uint32 familyIndex, uint32 queueIndex){
        uint32 queueCount = 0;
        if (graphicsFamilyIndex.has_value() && graphicsQueueIndex.has_value())
            queueCount += (graphicsFamilyIndex.value() == familyIndex) && (graphicsQueueIndex.value() == queueIndex);
        if (presentFamilyIndex.has_value() && presentQueueIndex.has_value())
            queueCount += (presentFamilyIndex.value() == familyIndex) && (presentQueueIndex.value() == queueIndex);
        if (transferFamilyIndex.has_value() && transferQueueIndex.has_value())
            queueCount += (transferFamilyIndex.value() == familyIndex) && (transferQueueIndex.value() == queueIndex);
        if (computeQueueIndex.has_value() && computeQueueIndex.has_value())
            queueCount += (computeQueueIndex.value() == familyIndex) && (computeQueueIndex.value() == queueIndex);

        return queueCount == 1;
    }

    uint32 getUniqueQueueCount(uint32 familyIndex){
        uint32 queuesInFamily = 0;
        // check if graphics in this family
        if (graphicsFamilyIndex.has_value() && (graphicsFamilyIndex.value() == familyIndex)){
            queuesInFamily++;
        }

        // check if present in this family
        if (presentFamilyIndex.has_value() && (presentFamilyIndex.value() == familyIndex)){
            queuesInFamily++;
        }

        // check if transfer in this family
        if (transferFamilyIndex.has_value() && (transferFamilyIndex.value() == familyIndex)){
            queuesInFamily++;
        }

        // check if compute in this family
        if (computeFamilyIndex.has_value() && (computeFamilyIndex.value() == familyIndex)){
            queuesInFamily++;
        }

        return queuesInFamily;
    }
} QueueFamilies;

}

