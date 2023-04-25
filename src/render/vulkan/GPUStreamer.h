#pragma once

#include "spruce_core.h"
#include "memory/Handle.h"
#include "memory/TempBuffer.h"
#include "resource/ResourceTypes.h"
#include "VulkanDevice.h"
#include "StagingBufferBatch.h"
#include "../../core/util/FunctionQueue.h"
#include "CommandBuffer.h"

namespace spr::gfx {
class GPUStreamer {
public:
    GPUStreamer();
    GPUStreamer(VulkanDevice& device, VulkanResourceManager& rm, CommandBuffer& transferCommandBuffer, CommandBuffer& graphicsCommandBuffer);
    ~GPUStreamer();
    
    template <typename T>
    void transfer(T data) {
        SprLog::warn("[GPUStreamer] Upload not available for buffer of this type");
    }

    template <typename T>
    void transferDynamic(T data, uint32 frame) {
        SprLog::warn("[GPUStreamer] Dynamic upload not available for buffer of this type");
    }

    void flush();

private:
    struct BufferTransfer {
        unsigned char* pSrc;
        uint32 size = 0;
        Buffer* dst;
        MemoryType memType = DEVICE;
    };

    struct TextureTransfer {
        unsigned char* pSrc;
        uint32 size = 0;
        Texture* dst;
    };

    // utility members (non-owning)
    VulkanDevice* m_device;
    VulkanResourceManager* m_rm;
    CommandBuffer* m_transferCommandBuffer;
    CommandBuffer* m_graphicsCommandBuffer;

    uint32 m_nonCoherentAtomSize;
    uint32 m_graphicsFamilyIndex;
    uint32 m_transferFamilyIndex;

    // 256 MB pre-allocated staging buffers
    StagingBuffers m_stagingBuffers;

    // cmd queues
    std::vector<std::function<VkBufferMemoryBarrier2()>> m_transferBufferBarriers;
    std::vector<std::function<VkBufferMemoryBarrier2()>> m_graphicsBufferBarriers;
    std::vector<std::function<VkImageMemoryBarrier2()>> m_imageLayoutBarriers;
    std::vector<std::function<VkImageMemoryBarrier2()>> m_transferImageBarriers;
    std::vector<std::function<VkImageMemoryBarrier2()>> m_graphicsImageBarriers;
    FunctionQueue m_bufferCopyCmdQueue;
    FunctionQueue m_imageCopyCmdQueue;

    bool m_destroyed = false;

    void init(VulkanDevice& device, VulkanResourceManager& rm, CommandBuffer& transferCommandBuffer, CommandBuffer& graphicsCommandBuffer);
    void reset();
    void performGraphicsBarriers();
    void destroy();

    friend class UploadHandler;
};

template<> void GPUStreamer::transfer(BufferTransfer data);
template<> void GPUStreamer::transfer(TextureTransfer data);
template<> void GPUStreamer::transferDynamic(BufferTransfer data, uint32 frame);

}