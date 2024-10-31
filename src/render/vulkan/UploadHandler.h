#pragma once

#include "gfx_vulkan_core.h"
#include "spruce_core.h"
#include "resource/ResourceTypes.h"
#include "GPUStreamer.h"
#include "external/volk/volk.h"

namespace spr::gfx {

class VulkanResourceManager;

class UploadHandler{
public:
    UploadHandler();
    UploadHandler& operator=(UploadHandler&& other) noexcept;
    UploadHandler& operator=(const UploadHandler& other);
    ~UploadHandler();

    template <typename T>
    void uploadBuffer(Span<T> src, Handle<Buffer> dst) {
        if (src.size() == 0)
            return;
        Buffer* dstBuffer = m_rm->get<Buffer>(dst);
        GPUStreamer::BufferTransfer transfer = {
            .pSrc = (unsigned char*)src.data(),
            .dst = dstBuffer,
            .size = (uint32)(src.size() * sizeof(T)),
            .memType = (MemoryType)dstBuffer->memType
        };
        m_streamer.transfer(transfer, false);
    }

    template <typename T>
    void uploadManagedBuffer(Handle<Buffer> src, Handle<Buffer> dst) {
        Buffer* dstBuffer = m_rm->get<Buffer>(dst);
        Buffer* srcBuffer = m_rm->get<Buffer>(src);
        GPUStreamer::BufferTransfer transfer = {
            .src = srcBuffer,
            .dst = dstBuffer,
            .size = (uint32)(srcBuffer->byteSize),
            .memType = (MemoryType)dstBuffer->memType
        };
        m_streamer.transfer(transfer, true);
    }

    template <typename T>
    void uploadDyanmicBuffer(Span<T> src, Handle<Buffer> dst) {
        if (src.size() == 0)
            return;
        Buffer* dstBuffer = m_rm->get<Buffer>(dst);
        GPUStreamer::BufferTransfer transfer = {
            .pSrc = (unsigned char*)src.data(),
            .dst = dstBuffer,
            .size = (uint32)(src.size() * sizeof(T)),
            .memType = (MemoryType)dstBuffer->memType
        };
        m_streamer.transferDynamic(transfer, m_frameId);
    }

    template <typename T>
    void uploadSparseBuffer(Span<T> src, Handle<Buffer> dst, uint32 srcOffset, uint32 dstOffset) {
        if (src.size() == 0)
            return;
        Buffer* dstBuffer = m_rm->get<Buffer>(dst);
        GPUStreamer::SparseBufferTransfer transfer = {
            .pSrc = (unsigned char*)src.data(),
            .size = sizeof(T),
            .dst = dstBuffer,
            .srcOffset = srcOffset,
            .dstOffset = dstOffset
        };
        m_streamer.transferDynamic(transfer, m_frameId);
    }

    template <typename T>
    void uploadTexture(Span<T> src, Handle<Texture> dst) {
        if (src.size() == 0)
            return;
        Texture* dstTexture = m_rm->get<Texture>(dst);
        GPUStreamer::TextureTransfer transfer = {
            .pSrc = (unsigned char*)src.data(),
            .dst = dstTexture,
            .size = (uint32)(src.size() * sizeof(T))
        };
        m_streamer.transfer(transfer, false);
    }

    template <typename T>
    void uploadManagedTexture(Handle<Buffer> src, Handle<Texture> dst) {
        Texture* dstTexture = m_rm->get<Texture>(dst);
        Buffer* srcBuffer = m_rm->get<Buffer>(src);
        GPUStreamer::TextureTransfer transfer = {
            .src = srcBuffer,
            .dst = dstTexture,
            .size = (uint32)srcBuffer->byteSize
        };
        m_streamer.transfer(transfer, true);
    }

    void submit();

private:
    GPUStreamer m_streamer;
    uint32 m_frameId;

    // non-owning
    VulkanResourceManager* m_rm;
    CommandBuffer* m_transferCommandBuffer;
    CommandBuffer* m_graphicsCommandBuffer;

    bool m_initialized = false;
    bool m_destroyed = false;

    void setFrameId(uint32 frameId);
    void init(VulkanDevice& device, VulkanResourceManager& rm, CommandBuffer& transferCommandBuffer, CommandBuffer& graphicsCommandBuffer);
    void reset();
    void destroy();
    void performGraphicsBarriers();

    friend class VulkanRenderer;
};
}