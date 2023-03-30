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
    std::vector<VkBufferMemoryBarrier2KHR> m_transferBufferBarriers;
    std::vector<VkBufferMemoryBarrier2KHR> m_graphicsBufferBarriers;
    std::vector<VkImageMemoryBarrier2KHR> m_imageLayoutBarriers;
    std::vector<VkImageMemoryBarrier2KHR> m_transferImageBarriers;
    std::vector<VkImageMemoryBarrier2KHR> m_graphicsImageBarriers;
    FunctionQueue m_bufferCopyCmdQueue;
    FunctionQueue m_imageCopyCmdQueue;

    void reset();
    void performGraphicsBarriers();

    friend class UploadHandler;
};
}