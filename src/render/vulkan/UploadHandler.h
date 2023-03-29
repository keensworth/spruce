#pragma once

#include "resource/VulkanResourceManager.h"
#include "vulkan_core.h"
#include "spruce_core.h"
#include "resource/ResourceTypes.h"
#include "../core/memory/TempBuffer.h"
#include "GPUStreamer.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

class UploadHandler{
public:
    UploadHandler();
    UploadHandler(VulkanDevice& device, VulkanResourceManager& rm, CommandBuffer& transferCommandBuffer, CommandBuffer& graphicsCommandBuffer);
    ~UploadHandler();

    template <typename T>
    void uploadBuffer(TempBuffer<T>& src, Handle<Buffer> dst) {
        m_bufferUploadQueue.push_back({
            .pSrc = src.getData(),
            .size = src.getSize(),
            .dst = dst
        });
    }

    template <typename T>
    void uploadDyanmicBuffer(TempBuffer<T>& src, Handle<Buffer> dst) {
        m_dynamicBufferUploadQueue.push_back({
            .pSrc = src.getData(),
            .size = src.getSize(),
            .dst = dst
        });
    }

    template <typename T>
    void uploadTexture(TempBuffer<T>& src, Handle<Texture> dst) {
        m_textureUploadQueue.push_back({
            .pSrc = src.getData(),
            .size = src.getSize(),
            .dst = dst
        });
    }

    void submit();

private:
    
    GPUStreamer m_streamer;
    uint32 m_frameId;

    CommandBuffer* m_transferCommandBuffer;
    CommandBuffer* m_graphicsCommandBuffer;

    std::vector<GPUStreamer::BufferTransfer> m_bufferUploadQueue;
    std::vector<GPUStreamer::BufferTransfer> m_dynamicBufferUploadQueue;
    std::vector<GPUStreamer::TextureTransfer> m_textureUploadQueue;

    void reset();
    void setFrameId(uint32 frameId);

    friend class VulkanRenderer;
};
}