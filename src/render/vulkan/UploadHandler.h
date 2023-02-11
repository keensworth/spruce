#pragma once

#include "vulkan_core.h"
#include "spruce_core.h"
#include "resource/ResourceTypes.h"
#include "../core/memory/TempBuffer.h"
#include "GPUStreamer.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

typedef struct BufferUpload {
    TempBuffer<uint8> src;
    Handle<Buffer> dst;
} BufferUpload;

typedef struct TextureUpload {
    TempBuffer<uint8> src;
    Handle<Texture> dst;
} TextureUpload;

class UploadHandler{
public:
    UploadHandler();
    UploadHandler(VulkanDevice& device);
    ~UploadHandler();

    void uploadBuffer(TempBuffer<uint8> src, Handle<Buffer> dst);
    void uploadDyanmicBuffer(TempBuffer<uint8> src, Handle<Buffer> dst);
    void uploadTexture(TempBuffer<uint8> src, Handle<Texture> dst);

private:
    GPUStreamer m_streamer;

    std::vector<BufferUpload> m_bufferUploadQueue;
    std::vector<BufferUpload> m_dynamicBufferUploadQueue;
    std::vector<TextureUpload> m_textureUploadQueue;

    void resetQueues();
    void submit(uint32 frame);

    friend class CommandBuffer;
};
}