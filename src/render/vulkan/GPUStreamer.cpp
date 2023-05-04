#include "GPUStreamer.h"

#include "StagingBufferBatch.h"
#include "VulkanDevice.h"
#include "resource/VulkanResourceManager.h"
#include <cstddef>
#include "../../external/volk/volk.h"
#include "../../debug/SprLog.h"
//extern struct VkBufferMemoryBarrier2;

namespace spr::gfx {

GPUStreamer::GPUStreamer() {
    
}

GPUStreamer::~GPUStreamer(){
    if (m_destroyed)
        return;
    
    SprLog::warn("[GPUStreamer] [~] Calling destroy() in destructor");
    destroy();
}

void GPUStreamer::init(VulkanDevice& device, VulkanResourceManager& rm, CommandBuffer& transferCommandBuffer, CommandBuffer& graphicsCommandBuffer){
    m_device = &device;
    m_rm = &rm;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceLimits limits;
    vkGetPhysicalDeviceProperties(m_device->getPhysicalDevice(), &properties);
    limits = properties.limits;
    m_nonCoherentAtomSize = limits.nonCoherentAtomSize;

    m_transferCommandBuffer = &transferCommandBuffer;
    m_graphicsCommandBuffer = &graphicsCommandBuffer;

    m_graphicsFamilyIndex = m_device->getQueueFamilies().graphicsFamilyIndex.has_value() ? m_device->getQueueFamilies().graphicsFamilyIndex.value() : 0;
    m_transferFamilyIndex = m_device->getQueueFamilies().transferFamilyIndex.has_value() ? m_device->getQueueFamilies().transferFamilyIndex.value() : 0;
    
    reset();

    m_stagingBuffers.init(&rm);
}

void GPUStreamer::destroy(){
    m_stagingBuffers.destroy();
    m_destroyed = true;
    SprLog::info("[GPUStreamer] [destroy] destroyed...");
}


template<>
void GPUStreamer::transfer(BufferTransfer data) {
    // shared, just upload
    if (data.memType == (HOST|DEVICE)) {
        std::memcpy(data.dst->allocInfo.pMappedData, data.pSrc, data.size);
        // https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
        uint32_t alignedSize = (data.size-1) - ((data.size-1) % m_nonCoherentAtomSize) + m_nonCoherentAtomSize;

        // build staging range and flush cache
        VkMappedMemoryRange stagingRange = {
            .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = data.dst->allocInfo.deviceMemory,
            .offset = 0,
            .size   = alignedSize
        };
        vkFlushMappedMemoryRanges(m_device->getDevice(), 1, &stagingRange);
        return;
    }

    // host local, just copy
    if (data.memType == HOST) {
        std::memcpy(data.dst->allocInfo.pMappedData, data.pSrc, data.size);
        return;
    }

    // device local, copy to staging buffer and upload
    Handle<Buffer> stagingBuffer = m_stagingBuffers.getStagingBuffer(data.size);
    Buffer* stage = m_rm->get<Buffer>(stagingBuffer);

    memcpy(stage->allocInfo.pMappedData, data.pSrc, data.size);

    // https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
    uint32_t alignedSize = (data.size-1) - ((data.size-1) % m_nonCoherentAtomSize) + m_nonCoherentAtomSize;

    // build staging range and flush cache
    VkMappedMemoryRange stagingRange = {
        .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = stage->allocInfo.deviceMemory,
        .offset = 0,
        .size   = alignedSize
    };
    vkFlushMappedMemoryRanges(m_device->getDevice(), 1, &stagingRange);

    // build copy region and perform copy command
    m_bufferCopyCmdQueue.push_function([=]() {
        VkBuffer stageBuffer = stage->buffer;
        VkBuffer dstBuffer = data.dst->buffer;
        uint32 dataSize = data.size;
        VkBufferCopy copyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size      = dataSize
        };
        vkCmdCopyBuffer(m_transferCommandBuffer->getCommandBuffer(), stageBuffer, dstBuffer, 1, &copyRegion);
    });

    // build barriers (transfer/graphics)
    m_transferBufferBarriers.push_back([=](){
        VkBuffer dstBuffer = data.dst->buffer;
        uint32 dataSize = data.size;
        VkBufferMemoryBarrier2KHR transferBarrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR,
            .pNext = NULL,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
            .srcQueueFamilyIndex = m_transferFamilyIndex,
            .dstQueueFamilyIndex = m_graphicsFamilyIndex,
            .buffer = dstBuffer,
            .offset = 0,
            .size   = dataSize
        };
        return transferBarrier;
    });

    m_graphicsBufferBarriers.push_back([=](){
        VkBuffer dstBuffer = data.dst->buffer;
        uint32 dataSize = data.size;
        VkBufferMemoryBarrier2KHR graphicsBarrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR,
            .pNext = NULL,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR,
            .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
            .srcQueueFamilyIndex = m_transferFamilyIndex,
            .dstQueueFamilyIndex = m_graphicsFamilyIndex,
            .buffer = dstBuffer,
            .offset = 0,
            .size   = dataSize
        };
        return graphicsBarrier;
    });
}

template<>
void GPUStreamer::transfer(TextureTransfer data) {
    // device local, copy to staging buffer and upload
    Handle<Buffer> stagingBuffer = m_stagingBuffers.getStagingBuffer(data.size);
    Buffer* stage = m_rm->get<Buffer>(stagingBuffer);
    std::memcpy(stage->allocInfo.pMappedData, data.pSrc, data.size);

    // https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
    uint32_t alignedSize = (data.size-1) - ((data.size-1) % m_nonCoherentAtomSize) + m_nonCoherentAtomSize;

    // build staging range and flush cache
    VkMappedMemoryRange stagingRange = {
        .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = stage->allocInfo.deviceMemory,
        .offset = 0,
        .size   = alignedSize
    };
    vkFlushMappedMemoryRanges(m_device->getDevice(), 1, &stagingRange);

    // build buffer image copy and perform copy command
    m_imageCopyCmdQueue.push_function([=](){
        VkBuffer stageBuffer = stage->buffer;
        VkImage dstImage = data.dst->image;
        uint32 aspectMask = data.dst->subresourceRange.aspectMask;
        uint32 mipLevel = data.dst->subresourceRange.baseMipLevel;
        uint32 baseArrayLayer = data.dst->subresourceRange.baseArrayLayer;
        uint32 layerCount = data.dst->subresourceRange.layerCount;
        VkExtent3D extent = {
            data.dst->dimensions.x,
            data.dst->dimensions.y,
            data.dst->dimensions.z};
        VkBufferImageCopy imageRegion = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = aspectMask,
                .mipLevel   = mipLevel,
                .baseArrayLayer = baseArrayLayer,
                .layerCount = layerCount
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = extent
        };
        vkCmdCopyBufferToImage(m_transferCommandBuffer->getCommandBuffer(), stageBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);
    });

    // build barriers (transfer/graphics)
    m_imageLayoutBarriers.push_back([=](){
        VkImage dstImage = data.dst->image;
        VkImageSubresourceRange subResourceRange = data.dst->subresourceRange;
        VkImageMemoryBarrier2KHR layoutTransitionBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR,
            .pNext = NULL,
            .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
            .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = dstImage,
            .subresourceRange = subResourceRange
        };
        return layoutTransitionBarrier;
    });

    
    m_transferImageBarriers.push_back([=](){
        VkImage dstImage = data.dst->image;
        VkImageSubresourceRange subResourceRange = data.dst->subresourceRange;
        VkImageMemoryBarrier2KHR transferImageBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR,
            .pNext = NULL,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
            .srcQueueFamilyIndex = m_transferFamilyIndex,
            .dstQueueFamilyIndex = m_graphicsFamilyIndex,
            .image = dstImage,
            .subresourceRange = subResourceRange
        };
        return transferImageBarrier;
    });

    
    m_graphicsImageBarriers.push_back([=](){
        VkImage dstImage = data.dst->image;
        VkImageSubresourceRange subResourceRange = data.dst->subresourceRange;
        VkImageMemoryBarrier2KHR graphicsImageBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2_KHR,
            .pNext = NULL,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT_KHR,
            .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
            .srcQueueFamilyIndex = m_transferFamilyIndex,
            .dstQueueFamilyIndex = m_graphicsFamilyIndex,
            .image = dstImage,
            .subresourceRange = subResourceRange
        };
        return graphicsImageBarrier;
    });
}

template<>
void GPUStreamer::transferDynamic(BufferTransfer data, uint32 frame) {
    uint32 offset = ((data.dst->byteSize)/MAX_FRAME_COUNT) * (frame % MAX_FRAME_COUNT);

    // shared, just upload
    if (data.memType == (HOST|DEVICE)) {
        std::memcpy(data.dst->allocInfo.pMappedData, data.pSrc + offset, data.size);
        // https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
        uint32_t alignedSize = (data.size-1) - ((data.size-1) % m_nonCoherentAtomSize) + m_nonCoherentAtomSize;

        // build staging range and flush cache
        VkMappedMemoryRange stagingRange = {
            .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
            .memory = data.dst->allocInfo.deviceMemory,
            .offset = 0,
            .size   = alignedSize
        };
        vkFlushMappedMemoryRanges(m_device->getDevice(), 1, &stagingRange);
        return;
    }

    // host local, just copy
    if (data.memType == HOST) {
        std::memcpy(data.dst->allocInfo.pMappedData, data.pSrc + offset, data.size);
        return;
    }

    // device local, copy to staging buffer and upload
    Handle<Buffer> stagingBuffer = m_stagingBuffers.getStagingBuffer(data.size);
    Buffer* stage = m_rm->get<Buffer>(stagingBuffer);
    std::memcpy(stage->allocInfo.pMappedData, data.pSrc, data.size);

    // https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
    uint32_t alignedSize = (data.size-1) - ((data.size-1) % m_nonCoherentAtomSize) + m_nonCoherentAtomSize;

    // build staging range and flush cache
    VkMappedMemoryRange stagingRange = {
        .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = stage->allocInfo.deviceMemory,
        .offset = offset,
        .size   = alignedSize
    };
    vkFlushMappedMemoryRanges(m_device->getDevice(), 1, &stagingRange);

    // build copy region and perform copy command
    m_bufferCopyCmdQueue.push_function([=]() {
        VkBuffer stageBuffer = stage->buffer;
        VkBuffer dstBuffer = data.dst->buffer;
        uint32 dataSize = data.size;
        uint32 dataOffset = offset;
        VkBufferCopy copyRegion = {
            .srcOffset = 0,
            .dstOffset = dataOffset,
            .size      = dataSize
        };
        vkCmdCopyBuffer(m_transferCommandBuffer->getCommandBuffer(), stageBuffer, dstBuffer, 1, &copyRegion);
    });

    // build barriers (transfer/graphics)
    
    m_transferBufferBarriers.push_back([=](){
        VkBuffer dstBuffer = data.dst->buffer;
        uint32 dataSize = data.size;
        VkBufferMemoryBarrier2KHR transferBarrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR,
            .pNext = NULL,
            .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
            .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
            .srcQueueFamilyIndex = m_transferFamilyIndex,
            .dstQueueFamilyIndex = m_graphicsFamilyIndex,
            .buffer = dstBuffer,
            .offset = offset,
            .size   = dataSize
        };
        return transferBarrier;
    });

    
    m_graphicsBufferBarriers.push_back([=](){
        VkBuffer dstBuffer = data.dst->buffer;
        uint32 dataSize = data.size;
        VkBufferMemoryBarrier2KHR graphicsBarrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2_KHR,
            .pNext = NULL,
            .dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT_KHR,
            .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
            .srcQueueFamilyIndex = m_transferFamilyIndex,
            .dstQueueFamilyIndex = m_graphicsFamilyIndex,
            .buffer = dstBuffer,
            .offset = offset,
            .size   = dataSize
        };
        return graphicsBarrier;
    });
}

void GPUStreamer::flush() {
    // buffer transfer barrier dependencies
    std::vector<VkBufferMemoryBarrier2KHR> transferBufferBarriers;
    for (const auto& barrierFunc : m_transferBufferBarriers) {
        VkBufferMemoryBarrier2KHR barrier = std::invoke(barrierFunc);
        transferBufferBarriers.push_back(barrier);
    }
    VkDependencyInfoKHR bufferTransferDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
        .pNext = NULL,
        .dependencyFlags = 0,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = NULL,
        .bufferMemoryBarrierCount = (uint32)transferBufferBarriers.size(),
        .pBufferMemoryBarriers = transferBufferBarriers.data(),
        .imageMemoryBarrierCount = 0,
        .pImageMemoryBarriers = NULL
    };

    // image layout transition dependencies
    std::vector<VkImageMemoryBarrier2KHR> imageLayoutBarriers;
    for (const auto& barrierFunc : m_imageLayoutBarriers) {
        VkImageMemoryBarrier2KHR barrier = std::invoke(barrierFunc);
        imageLayoutBarriers.push_back(barrier);
    }
    VkDependencyInfoKHR imageLayoutDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
        .pNext = NULL,
        .dependencyFlags = 0,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = NULL,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = NULL,
        .imageMemoryBarrierCount = (uint32)imageLayoutBarriers.size(),
        .pImageMemoryBarriers = imageLayoutBarriers.data()
    };

    // image transfer barrier dependencies
    std::vector<VkImageMemoryBarrier2KHR> transferImageBarriers;
    for (const auto& barrierFunc : m_transferImageBarriers) {
        VkImageMemoryBarrier2KHR barrier = std::invoke(barrierFunc);
        transferImageBarriers.push_back(barrier);
    }
    VkDependencyInfoKHR imageTransferDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
        .pNext = NULL,
        .dependencyFlags = 0,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = NULL,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = NULL,
        .imageMemoryBarrierCount = (uint32)transferImageBarriers.size(),
        .pImageMemoryBarriers = transferImageBarriers.data()
    };

    // perform buffer copy commands
    m_bufferCopyCmdQueue.execute();

    // perform buffer transfer queue pipeline barrier
    vkCmdPipelineBarrier2KHR(m_transferCommandBuffer->getCommandBuffer(), &bufferTransferDependencies);

    // // perform image layout pipeline barrier
    vkCmdPipelineBarrier2KHR(m_transferCommandBuffer->getCommandBuffer(), &imageLayoutDependencies);

    // perform buffer->image copy commands
    m_imageCopyCmdQueue.execute();

    // perform image transfer queue pipeline barrier
    vkCmdPipelineBarrier2KHR(m_transferCommandBuffer->getCommandBuffer(), &imageTransferDependencies);
}

void GPUStreamer::performGraphicsBarriers() {
    // buffer graphics barrier dependencies
    std::vector<VkBufferMemoryBarrier2KHR> graphicsBufferBarriers;
    for (const auto& barrierFunc : m_graphicsBufferBarriers) {
        VkBufferMemoryBarrier2KHR barrier = std::invoke(barrierFunc);
        graphicsBufferBarriers.push_back(barrier);
    }
    VkDependencyInfoKHR bufferGraphicsDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
        .pNext = NULL,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = NULL,
        .bufferMemoryBarrierCount = (uint32)graphicsBufferBarriers.size(),
        .pBufferMemoryBarriers = graphicsBufferBarriers.data(),
        .imageMemoryBarrierCount = 0,
        .pImageMemoryBarriers = NULL
    };

    // image graphics barrier dependencies
    std::vector<VkImageMemoryBarrier2KHR> graphicsImageBarriers;
    for (const auto& barrierFunc : m_graphicsImageBarriers) {
        VkImageMemoryBarrier2KHR barrier = std::invoke(barrierFunc);
        graphicsImageBarriers.push_back(barrier);
    }
    VkDependencyInfoKHR imageGraphicsDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
        .pNext = NULL,
        .dependencyFlags = 0,
        .memoryBarrierCount = 0,
        .pMemoryBarriers = NULL,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = NULL,
        .imageMemoryBarrierCount = (uint32)graphicsImageBarriers.size(),
        .pImageMemoryBarriers = graphicsImageBarriers.data()
    };

    // perform buffer transfer queue pipeline barrier
    vkCmdPipelineBarrier2KHR(m_graphicsCommandBuffer->getCommandBuffer(), &bufferGraphicsDependencies);

    // perform image transfer queue pipeline barrier
    vkCmdPipelineBarrier2KHR(m_graphicsCommandBuffer->getCommandBuffer(), &imageGraphicsDependencies);
}

void GPUStreamer::reset() {
    m_transferBufferBarriers.clear();
    m_graphicsBufferBarriers.clear();
    m_imageLayoutBarriers.clear();
    m_transferImageBarriers.clear();
    m_graphicsImageBarriers.clear();

    m_stagingBuffers.reset();
}

}
