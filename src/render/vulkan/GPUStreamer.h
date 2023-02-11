#pragma once

#include "spruce_core.h"
#include "memory/Handle.h"
#include "memory/TempBuffer.h"
#include "resource/ResourceTypes.h"
#include "VulkanDevice.h"

namespace spr::gfx {
class GPUStreamer {
public:
    GPUStreamer();
    GPUStreamer(VulkanDevice& device);
    ~GPUStreamer();
    
    template <typename T, typename U>
    void upload(TempBuffer<T> src, Handle<U> dst) {
        SprLog::warn("GPUStreamer: Upload not available for buffer of this type");
    }

    template <typename T, typename U>
    void uploadDynamic(TempBuffer<T> src, Handle<U> dst, uint32 frame) {
        SprLog::warn("GPUStreamer: Dynamic upload not available for buffer of this type");
    }

private:
    VulkanDevice* m_device;

};
}