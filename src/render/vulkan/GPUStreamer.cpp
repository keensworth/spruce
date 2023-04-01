#include "GPUStreamer.h"

#include "StagingBufferBatch.h"
#include "VulkanDevice.h"
#include "resource/VulkanResourceManager.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

GPUStreamer::GPUStreamer() {
    
}

GPUStreamer::GPUStreamer(VulkanDevice& device, VulkanResourceManager& rm, CommandBuffer& transferCommandBuffer, CommandBuffer& graphicsCommandBuffer)
                        : m_stagingBuffers(&rm){
    m_device = &device;
    m_rm = &rm;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceLimits limits;
    vkGetPhysicalDeviceProperties(m_device->getPhysicalDevice(), &properties);
    limits = properties.limits;
    m_nonCoherentAtomSize = limits.nonCoherentAtomSize;

    m_transferCommandBuffer = &transferCommandBuffer;
    m_graphicsCommandBuffer = &graphicsCommandBuffer;

    m_graphicsFamilyIndex = m_device->getQueueFamilies().graphicsFamilyIndex.value();
    m_transferFamilyIndex = m_device->getQueueFamilies().transferFamilyIndex.value();
    
    reset();
}

GPUStreamer::~GPUStreamer(){
    if (m_destroyed)
        return;
    
    SprLog::warn("[GPUStreamer] [~] Calling destroy() in destructor");
    destroy();
}

void GPUStreamer::destroy(){
    m_stagingBuffers.destroy();
    m_destroyed = true;
}


template<>
void GPUStreamer::transfer(BufferTransfer data) {
    // shared, just upload
    if (data.memType == (HOST|DEVICE)) {
        memcpy(data.dst->allocInfo.pMappedData, data.pSrc, data.size);
        return;
    }

    // host local, just copy
    if (data.memType == HOST) {
        memcpy(data.dst->allocInfo.pMappedData, data.pSrc, data.size);
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
        .memory = data.dst->allocInfo.deviceMemory,
        .offset = data.dst->allocInfo.offset,
        .size   = alignedSize
    };
    vkFlushMappedMemoryRanges(m_device->getDevice(), 1, &stagingRange);

    // build copy region and perform copy command
    m_bufferCopyCmdQueue.push_function([=]() {
        VkBufferCopy copyRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size      = data.size
        };
        vkCmdCopyBuffer(m_transferCommandBuffer->getCommandBuffer(), stage->buffer, data.dst->buffer, 1, &copyRegion);
    });

    // build barriers (transfer/graphics)
    VkBufferMemoryBarrier2KHR transferBarrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
        .srcQueueFamilyIndex = m_transferFamilyIndex,
        .dstQueueFamilyIndex = m_graphicsFamilyIndex,
        .buffer = data.dst->buffer,
        .offset = 0,
        .size   = data.size
    };
    m_transferBufferBarriers.push_back(transferBarrier);

    VkBufferMemoryBarrier2KHR graphicsBarrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
        .srcQueueFamilyIndex = m_transferFamilyIndex,
        .dstQueueFamilyIndex = m_graphicsFamilyIndex,
        .buffer = data.dst->buffer,
        .offset = 0,
        .size   = data.size
    };
    m_graphicsBufferBarriers.push_back(graphicsBarrier);
}

template<>
void GPUStreamer::transfer(TextureTransfer data) {
    // device local, copy to staging buffer and upload
    Handle<Buffer> stagingBuffer = m_stagingBuffers.getStagingBuffer(data.size);
    Buffer* stage = m_rm->get<Buffer>(stagingBuffer);
    memcpy(stage->allocInfo.pMappedData, data.pSrc, data.size);

    // https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
    uint32_t alignedSize = (data.size-1) - ((data.size-1) % m_nonCoherentAtomSize) + m_nonCoherentAtomSize;

    // build staging range and flush cache
    VkMappedMemoryRange stagingRange = {
        .sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = data.dst->allocInfo.deviceMemory,
        .offset = data.dst->allocInfo.offset,
        .size   = alignedSize
    };
    vkFlushMappedMemoryRanges(m_device->getDevice(), 1, &stagingRange);

    // build buffer image copy and perform copy command
    m_imageCopyCmdQueue.push_function([=](){
        VkBufferImageCopy imageRegion = {
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                .aspectMask = data.dst->subresourceRange.aspectMask,
                .mipLevel   = data.dst->subresourceRange.baseMipLevel,
                .baseArrayLayer = data.dst->subresourceRange.baseArrayLayer,
                .layerCount = data.dst->subresourceRange.layerCount
            },
            .imageOffset = {0, 0, 0},
            .imageExtent = {
                data.dst->dimensions.x,
                data.dst->dimensions.y,
                data.dst->dimensions.z,
            }
        };
        vkCmdCopyBufferToImage(m_transferCommandBuffer->getCommandBuffer(), stage->buffer, data.dst->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);
    });

    // build barriers (transfer/graphics)
    VkImageMemoryBarrier2KHR layoutTransitionBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = data.dst->image,
        .subresourceRange = data.dst->subresourceRange
    };
    m_imageLayoutBarriers.push_back(layoutTransitionBarrier);

    VkImageMemoryBarrier2KHR transferImageBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
        .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = m_transferFamilyIndex,
        .dstQueueFamilyIndex = m_graphicsFamilyIndex,
        .image = data.dst->image,
        .subresourceRange = data.dst->subresourceRange
    };
    m_transferImageBarriers.push_back(transferImageBarrier);

    VkImageMemoryBarrier2KHR graphicsImageBarrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT_KHR,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
        .srcQueueFamilyIndex = m_transferFamilyIndex,
        .dstQueueFamilyIndex = m_graphicsFamilyIndex,
        .image = data.dst->image,
        .subresourceRange = data.dst->subresourceRange
    };
    m_graphicsImageBarriers.push_back(graphicsImageBarrier);
}

template<>
void GPUStreamer::transferDynamic(BufferTransfer data, uint32 frame) {
    uint32 offset = ((data.dst->byteSize)/MAX_FRAME_COUNT) * (frame % MAX_FRAME_COUNT);

    // shared, just upload
    if (data.memType == (HOST|DEVICE)) {
        memcpy(data.dst->allocInfo.pMappedData, data.pSrc + offset, data.size);
        return;
    }

    // host local, just copy
    if (data.memType == HOST) {
        memcpy(data.dst->allocInfo.pMappedData, data.pSrc + offset, data.size);
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
        .memory = data.dst->allocInfo.deviceMemory,
        .offset = data.dst->allocInfo.offset + offset,
        .size   = alignedSize
    };
    vkFlushMappedMemoryRanges(m_device->getDevice(), 1, &stagingRange);

    // build copy region and perform copy command
    m_bufferCopyCmdQueue.push_function([=]() {
        VkBufferCopy copyRegion = {
            .srcOffset = 0,
            .dstOffset = offset,
            .size      = data.size
        };
        vkCmdCopyBuffer(m_transferCommandBuffer->getCommandBuffer(), stage->buffer, data.dst->buffer, 1, &copyRegion);
    });

    // build barriers (transfer/graphics)
    VkBufferMemoryBarrier2KHR transferBarrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
        .srcQueueFamilyIndex = m_transferFamilyIndex,
        .dstQueueFamilyIndex = m_graphicsFamilyIndex,
        .buffer = data.dst->buffer,
        .offset = offset,
        .size   = data.size
    };
    m_transferBufferBarriers.push_back(transferBarrier);

    VkBufferMemoryBarrier2KHR graphicsBarrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
        .srcQueueFamilyIndex = m_transferFamilyIndex,
        .dstQueueFamilyIndex = m_graphicsFamilyIndex,
        .buffer = data.dst->buffer,
        .offset = offset,
        .size   = data.size
    };
    m_graphicsBufferBarriers.push_back(graphicsBarrier);
}

void GPUStreamer::flush() {
    // buffer transfer barrier dependencies
    VkDependencyInfoKHR bufferTransferDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = 0,
        .bufferMemoryBarrierCount = (uint32)m_transferBufferBarriers.size(),
        .pBufferMemoryBarriers = m_transferBufferBarriers.data()
    };

    // image layout transition dependencies
    VkDependencyInfoKHR imageLayoutDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = 0,
        .imageMemoryBarrierCount = (uint32)m_imageLayoutBarriers.size(),
        .pImageMemoryBarriers = m_imageLayoutBarriers.data()
    };

    // image transfer barrier dependencies
    VkDependencyInfoKHR imageTransferDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = 0,
        .imageMemoryBarrierCount = (uint32)m_transferImageBarriers.size(),
        .pImageMemoryBarriers = m_transferImageBarriers.data()
    };

    // perform buffer copy commands
    m_bufferCopyCmdQueue.execute();

    // perform buffer transfer queue pipeline barrier
    vkCmdPipelineBarrier2KHR(m_transferCommandBuffer->getCommandBuffer(), &bufferTransferDependencies);

    // perform image layout pipeline barrier
    vkCmdPipelineBarrier2KHR(m_transferCommandBuffer->getCommandBuffer(), &imageLayoutDependencies);

    // perform buffer->image copy commands
    m_imageCopyCmdQueue.execute();

    // perform image transfer queue pipeline barrier
    vkCmdPipelineBarrier2KHR(m_transferCommandBuffer->getCommandBuffer(), &imageTransferDependencies);
}

void GPUStreamer::performGraphicsBarriers() {
    // buffer graphics barrier dependencies
    VkDependencyInfoKHR bufferGraphicsDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = 0,
        .bufferMemoryBarrierCount = (uint32)m_graphicsBufferBarriers.size(),
        .pBufferMemoryBarriers = m_graphicsBufferBarriers.data()
    };

    // image graphics barrier dependencies
    VkDependencyInfoKHR imageGraphicsDependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .dependencyFlags = 0,
        .imageMemoryBarrierCount = (uint32)m_graphicsImageBarriers.size(),
        .pImageMemoryBarriers = m_graphicsImageBarriers.data()
    };

    // perform buffer transfer queue pipeline barrier
    vkCmdPipelineBarrier2KHR(m_transferCommandBuffer->getCommandBuffer(), &bufferGraphicsDependencies);

    // perform image transfer queue pipeline barrier
    vkCmdPipelineBarrier2KHR(m_transferCommandBuffer->getCommandBuffer(), &imageGraphicsDependencies);
}

void GPUStreamer::reset() {
    m_transferBufferBarriers = std::vector<VkBufferMemoryBarrier2KHR>();
    m_transferBufferBarriers.reserve(32);
    m_graphicsBufferBarriers = std::vector<VkBufferMemoryBarrier2KHR>();
    m_graphicsBufferBarriers.reserve(32);
    m_imageLayoutBarriers = std::vector<VkImageMemoryBarrier2KHR>();
    m_imageLayoutBarriers.reserve(32);
    m_transferImageBarriers = std::vector<VkImageMemoryBarrier2KHR>();
    m_transferImageBarriers.reserve(32);
    m_graphicsImageBarriers = std::vector<VkImageMemoryBarrier2KHR>();
    m_graphicsImageBarriers.reserve(32);

    m_stagingBuffers.reset();
}

}
