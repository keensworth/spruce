#pragma once

#include "gfx_vulkan_core.h"
#include "VulkanDevice.h"

namespace spr {
    class SprWindow;
}

namespace spr::gfx{

struct TranscodeResult {
    VkFormat format;
    uint32 mips;
    uint32 sizeBytes;
    uint8* transcodedData;
};

class TextureTranscoder{
public:
    TextureTranscoder();
    TextureTranscoder(VulkanDevice* device);
    ~TextureTranscoder();

    TranscodeResult transcode(uint8* data, uint32 size, uint32 width, uint32 height);
    bool formatSupported(VkFormat format);

private:
    VulkanDevice* m_device;
    VkPhysicalDeviceFeatures m_features;

    struct FormatSupport {
        bool ETC2 = false;
        bool BC7 = false;
        bool BC4 = false;
        bool BC3 = false;
        bool BC1 = false;
        bool PVRTC1 = false;
        bool ASTC = false;
    };

    FormatSupport m_supported;
};
}