#pragma once

#include "gfx_vulkan_core.h"
#include "core/util/FunctionQueue.h"
#include "core/memory/TempBuffer.h"
#include "ktx.h"
#include "vulkan/resource/OffsetBuffer.h"

namespace spr {
    class SprWindow;
}

namespace spr::gfx{

typedef OffsetBuffer OffsetBuffer;
class VulkanDevice;

struct TranscodeResult {
    VkFormat format;
    uint32 mips;
    uint32 layers;
    uint32 sizeBytes;
    OffsetBuffer transcodedData;
};

class TextureTranscoder{
public:
    TextureTranscoder();
    TextureTranscoder(VulkanDevice* device);
    ~TextureTranscoder();

    void transcode(TranscodeResult& out, VulkanResourceManager* vrm, uint8* data, uint32 size, uint32 width, uint32 height);
    void destroyActiveTexture(TranscodeResult& out, uint32 sizeBytes);
    void reset();
    bool formatSupported(VkFormat format);

private:
    VulkanDevice* m_device;
    VkPhysicalDeviceFeatures m_features;
    ktxTexture2* m_activeTexture;

    FunctionQueue m_deletionQueue;

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