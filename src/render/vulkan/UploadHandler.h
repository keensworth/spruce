#pragma once

#include "resource/VulkanResourceManager.h"
#include "gfx_vulkan_core.h"
#include "spruce_core.h"
#include "resource/ResourceTypes.h"
#include "../core/memory/TempBuffer.h"
#include "GPUStreamer.h"
#include "../../external/volk/volk.h"

namespace spr::gfx {

class UploadHandler{
public:
    UploadHandler();
    UploadHandler& operator=(UploadHandler&& other) noexcept;
    UploadHandler& operator=(const UploadHandler& other);
    ~UploadHandler();




    template <typename T>
    void uploadBuffer(TempBuffer<T>& src, Handle<Buffer> dst) {
        if (src.getSize() == 0)
            return;
        Buffer* dstBuffer = m_rm->get<Buffer>(dst);
        GPUStreamer::BufferTransfer transfer = {
            .pSrc = (unsigned char*)src.getBytes(),
            .size = (uint32)(src.getSize() * sizeof(T)),
            .dst = dstBuffer,
            .memType = (MemoryType)dstBuffer->memType
        };
        m_bufferUploadQueue.push_back(transfer);
    }

    template <typename T>
    void uploadDyanmicBuffer(TempBuffer<T>& src, Handle<Buffer> dst) {
        if (src.getSize() == 0)
            return;
        Buffer* dstBuffer = m_rm->get<Buffer>(dst);
        GPUStreamer::BufferTransfer transfer = {
            .pSrc = (unsigned char*)src.getBytes(),
            .size = (uint32)(src.getSize() * sizeof(T)),
            .dst = dstBuffer,
            .memType = (MemoryType)dstBuffer->memType
        };
        m_dynamicBufferUploadQueue.push_back(transfer);
    }

    template <typename T>
    void uploadSparseBuffer(TempBuffer<T>& src, Handle<Buffer> dst, uint32 srcOffset, uint32 dstOffset) {
        if (src.getSize() == 0)
            return;
        Buffer* dstBuffer = m_rm->get<Buffer>(dst);
        GPUStreamer::SparseBufferTransfer transfer = {
            .pSrc = (unsigned char*)src.getBytes(),
            .size = sizeof(T),
            .dst = dstBuffer,
            .srcOffset = srcOffset,
            .dstOffset = dstOffset
        };
        m_sparseBufferUploadQueue.push_back(transfer);
    }

    template <typename T>
    void uploadTexture(TempBuffer<T>& src, Handle<Texture> dst) {
        if (src.getSize() == 0)
            return;
        Texture* dstTexture = m_rm->get<Texture>(dst);
        GPUStreamer::TextureTransfer transfer = {
            .pSrc = (unsigned char*)src.getBytes(),
            .size = (uint32)(src.getSize() * sizeof(T)),
            .dst = dstTexture,
        };
        m_textureUploadQueue.push_back(transfer);
    }

    void submit();

private:
    GPUStreamer m_streamer;
    uint32 m_frameId;

    // non-owning
    VulkanResourceManager* m_rm;
    CommandBuffer* m_transferCommandBuffer;
    CommandBuffer* m_graphicsCommandBuffer;

    std::vector<GPUStreamer::BufferTransfer> m_bufferUploadQueue;
    std::vector<GPUStreamer::BufferTransfer> m_dynamicBufferUploadQueue;
    std::vector<GPUStreamer::SparseBufferTransfer> m_sparseBufferUploadQueue;
    std::vector<GPUStreamer::TextureTransfer> m_textureUploadQueue;

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