#include "CommandBuffer.h"

#include "vulkan/VulkanDevice.h"
#include "RenderPassRenderer.h"
#include "UploadHandler.h"
#include <bits/ranges_base.h>
#include "external/volk/volk.h"
#include "debug/SprLog.h"

namespace spr::gfx {

CommandBuffer::CommandBuffer() {}

CommandBuffer::~CommandBuffer(){
    if (m_destroyed || !m_initialized)
        return;
    
    SprLog::warn("[CommandBuffer] [~] Calling destroy() in destructor");
    destroy();
}

void CommandBuffer::init(VulkanDevice& device, VulkanResourceManager* rm,uint32 frameIndex, CommandType commandType, VkCommandBuffer commandBuffer, VkQueue queue){
    m_rm = rm;
    m_device = &device;
    m_type = commandType;
    m_commandBuffer = commandBuffer;
    m_queue = queue;
    m_frameId = 0;
    m_frameIndex = frameIndex;

    m_passRenderer = RenderPassRenderer(m_rm, commandBuffer, frameIndex);

    // build semaphore info and create semaphore
    VkSemaphoreCreateInfo semaphoreInfo { 
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0
    };
    VK_CHECK(vkCreateSemaphore(m_device->getDevice(), &semaphoreInfo, NULL, &m_semaphore));

    // build fence info and create fence
    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    VK_CHECK(vkCreateFence(m_device->getDevice(), &fenceInfo, NULL, &m_fence));
    m_initialized = true;
}

void CommandBuffer::destroy(){
    // teardown own sync structures
    vkDestroyFence(m_device->getDevice(), m_fence, nullptr);
    vkDestroySemaphore(m_device->getDevice(), m_semaphore, nullptr);
    m_destroyed = true;
    SprLog::info("[CommandBuffer] [destroy] destroyed...");
}


void CommandBuffer::begin(){
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
    m_recording = true;
}

void CommandBuffer::end(){
    VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
    m_recording = false;
}


RenderPassRenderer& CommandBuffer::beginRenderPass(Handle<RenderPass> handle, glm::vec4 clearColor){
    // make sure user is accessing correct commandbuffer
    if (m_type != CommandType::OFFSCREEN && m_type != CommandType::MAIN){
        SprLog::warn("[CommandBuffer] Not a render command buffer");
    }

    // get attachment counts (color + depth)
    RenderPass* renderPass = m_rm->get<RenderPass>(handle);
    Framebuffer* framebuffer = m_rm->get<Framebuffer>(renderPass->framebuffer);
    bool hasDepth = framebuffer->hasDepthAttachment;
    uint32 colorCount = framebuffer->colorAttachments.size();

    // create clear values and begin renderpass
    std::vector<VkClearValue> clearValues(colorCount + hasDepth);
    for (uint32 i = 0; i < colorCount; i++)
        clearValues[i] = VkClearValue {
            .color = {{clearColor.x, clearColor.y, clearColor.z, clearColor.w}} // (green-gray by default)
        };
    if (hasDepth){
        uint32 compareOp = framebuffer->depthAttachment.compareOp;
        VkClearDepthStencilValue depthClearColor = {1.0f, 0};
        if (compareOp == Flags::Compare::GREATER || compareOp == Flags::Compare::GREATER_OR_EQUAL)
            depthClearColor = {0.0f, 0};
        clearValues[colorCount] = VkClearValue {
            .depthStencil = depthClearColor
        };
    } 

    VkRenderPassBeginInfo renderPassInfo {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass      = renderPass->renderPass,
        .framebuffer     = framebuffer->framebuffers[m_frameId % MAX_FRAME_COUNT],
        .renderArea      = {
            .offset = {0,0},
            .extent = {renderPass->dimensions.x, renderPass->dimensions.y}
        },
        .clearValueCount = (uint32)clearValues.size(),
        .pClearValues    = clearValues.data(),
    };
    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    m_passRenderer.setDimensions(renderPass->dimensions);

    // prepare and return render pass renderer
    return m_passRenderer;
}

RenderPassRenderer& CommandBuffer::beginRenderPass(Handle<RenderPass> renderPassHandle, Handle<Framebuffer> framebufferHandle, glm::vec4 clearColor){
    // make sure user is accessing correct commandbuffer
    if (m_type != CommandType::OFFSCREEN && m_type != CommandType::MAIN){
        SprLog::warn("[CommandBuffer] Not a render command buffer");
    }

    // get attachment counts (color + depth)
    RenderPass* renderPass = m_rm->get<RenderPass>(renderPassHandle);
    Framebuffer* framebuffer = m_rm->get<Framebuffer>(framebufferHandle);
    bool hasDepth = framebuffer->hasDepthAttachment;
    uint32 colorCount = framebuffer->colorAttachments.size();

    // create clear values and begin renderpass
    std::vector<VkClearValue> clearValues(colorCount + hasDepth);
    for (uint32 i = 0; i < colorCount; i++)
        clearValues[i] = VkClearValue {
            .color = {{clearColor.x, clearColor.y, clearColor.z, clearColor.w}} // (green-gray by default)
        };
    if (hasDepth){
        uint32 compareOp = framebuffer->depthAttachment.compareOp;
        VkClearDepthStencilValue depthClearColor = {1.0f, 0};
        if (compareOp == Flags::Compare::GREATER || compareOp == Flags::Compare::GREATER_OR_EQUAL)
            depthClearColor = {0.0f, 0};
        clearValues[colorCount] = VkClearValue {
            .depthStencil = depthClearColor
        };
    } 

    VkRenderPassBeginInfo renderPassInfo {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass      = renderPass->renderPass,
        .framebuffer     = framebuffer->framebuffers[m_frameId % MAX_FRAME_COUNT],
        .renderArea      = {
            .offset = {0,0},
            .extent = {framebuffer->dimensions.x, framebuffer->dimensions.y}
        },
        .clearValueCount = (uint32)clearValues.size(),
        .pClearValues    = clearValues.data(),
    };
    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_passRenderer.setDimensions(framebuffer->dimensions);

    // prepare and return render pass renderer
    return m_passRenderer;
}

void CommandBuffer::endRenderPass(){
    vkCmdEndRenderPass(m_commandBuffer);
}

RenderPassRenderer& CommandBuffer::beginComputePass(){
    // make sure user is accessing correct commandbuffer
    if (m_type != CommandType::OFFSCREEN && m_type != CommandType::MAIN){
        SprLog::warn("[CommandBuffer] Not a render command buffer");
    }

    return m_passRenderer;
}

void CommandBuffer::endComputePass(){
    VkMemoryBarrier2KHR memoryBarrier {
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2_KHR,
        .pNext = NULL,
        .srcStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT
    };

    VkDependencyInfoKHR dependencies = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO_KHR,
        .pNext = NULL,
        .dependencyFlags = 0,
        .memoryBarrierCount = 1,
        .pMemoryBarriers = &memoryBarrier,
        .bufferMemoryBarrierCount = 0,
        .pBufferMemoryBarriers = NULL,
        .imageMemoryBarrierCount = 0,
        .pImageMemoryBarriers = NULL
    };

    vkCmdPipelineBarrier2KHR(m_commandBuffer, &dependencies);
}

void CommandBuffer::bindIndexBuffer(Handle<Buffer> indexBuffer){
    Buffer* buffer = m_rm->get<Buffer>(indexBuffer);
    vkCmdBindIndexBuffer(m_commandBuffer, buffer->buffer, 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::submit(){
    std::vector<VkPipelineStageFlags> stageFlags;

    for (uint32 i = 0; i < m_waitSemaphores.size(); i++){
        if (m_type == CommandType::TRANSFER) {
            stageFlags.push_back(VK_PIPELINE_STAGE_TRANSFER_BIT);
        } else {
            stageFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }
    }

    end();

    VkSubmitInfo submitInfo {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = (uint32)m_waitSemaphores.size(),
        .pWaitSemaphores      = m_waitSemaphores.data(),
        .pWaitDstStageMask    = stageFlags.data(),
        .commandBufferCount   = 1,
        .pCommandBuffers      = &m_commandBuffer,
        .signalSemaphoreCount = (uint32)m_signalSemaphores.size(),
        .pSignalSemaphores    = m_signalSemaphores.data(),
    };

    VK_CHECK(vkQueueSubmit(m_queue, 1, &submitInfo, m_fence));
    m_fenceInUse = true;
}


VkCommandBuffer CommandBuffer::getCommandBuffer(){
    return m_commandBuffer;
}

VkSemaphore CommandBuffer::getSemaphore(){
    return m_semaphore;
}

VkFence CommandBuffer::getFence(){
    return m_fence;
}

bool CommandBuffer::isRecording(){
    return m_recording;
}


void CommandBuffer::waitFence(){
    if (!m_initialized)
        SprLog::error("[CommandBuffer] [waitFence] CB not initialized");
    if (!m_fenceInUse)
        return;
    VK_CHECK(vkWaitForFences(m_device->getDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX));
    m_fenceInUse = false;
}

void CommandBuffer::resetFence(){
    VK_CHECK(vkResetFences(m_device->getDevice(), 1, &m_fence));
    m_fenceInUse = false;
}


void CommandBuffer::setSemaphoreDependencies(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores){
    m_waitSemaphores = waitSemaphores;
    m_signalSemaphores = signalSemaphores;
}


void CommandBuffer::setFrameId(uint32 frameId) {
    m_frameId = frameId;
    uint32 frameIndex = m_frameId % MAX_FRAME_COUNT;
    if (frameIndex != m_frameIndex){
        std::string errMsg("[CommandBuffer] Frame Index mismatch! ");
        std::string errInfo("Expected: " + std::to_string(m_frameIndex) + ", got: " + std::to_string(frameIndex));
        SprLog::error(errMsg + errInfo);
    }

    m_passRenderer.setFrameId(frameId);
}

}