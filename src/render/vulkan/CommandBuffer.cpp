#include "CommandBuffer.h"

#include "RenderPassRenderer.h"
#include "UploadHandler.h"
#include "vulkan_core.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

CommandBuffer::CommandBuffer() {}

CommandBuffer::CommandBuffer(VulkanDevice& device, CommandType commandType, VkCommandBuffer commandBuffer, VulkanResourceManager* rm, VkQueue queue, uint32 frameIndex){
    m_type = commandType;
    m_commandBuffer = commandBuffer;
    m_rm = rm;
    m_device = &device;
    m_queue = queue;
    m_frameId = 0;
    m_frameIndex = frameIndex;

    // build semaphore info and create semaphore
    VkSemaphoreCreateInfo semaphoreInfo { 
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VK_CHECK(vkCreateSemaphore(device.getDevice(), &semaphoreInfo, NULL, &m_semaphore));

    // build fence info and create fence
    VkFenceCreateInfo fenceInfo {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };
    VK_CHECK(vkCreateFence(device.getDevice(), &fenceInfo, NULL, &m_fence));

    // create render pass renderer
    m_passRenderer = RenderPassRenderer(rm, commandBuffer, frameIndex);
}

CommandBuffer::~CommandBuffer(){
    if (!m_destroyed)
        SprLog::error("[CommandBuffer] [~] 'destroy' must be called before destructing - Improper release of resources");
}

void CommandBuffer::destroy(){
    // teardown own sync structures
    vkDestroyFence(m_device->getDevice(), m_fence, nullptr);
    vkDestroySemaphore(m_device->getDevice(), m_semaphore, nullptr);
    m_destroyed = true;
}


void CommandBuffer::begin(){
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void CommandBuffer::end(){
    VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
}


RenderPassRenderer& CommandBuffer::beginRenderPass(Handle<RenderPass> handle, glm::vec4 clearColor = {0.45098f, 0.52549f, 0.47058f, 1.0f}){
    // make sure user is accessing correct commandbuffer
    if (m_type != CommandType::OFFSCREEN && m_type != CommandType::MAIN){
        SprLog::warn("[CommandBuffer] Not a render command buffer");
    }

    // get attachment counts (color + depth)
    RenderPass* renderPass = m_rm->get<RenderPass>(handle);
    bool hasDepth = renderPass->hasDepthAttachment;
    uint32 colorCount = renderPass->colorAttachments.size();

    // create clear values and begin renderpass
    std::vector<VkClearValue> clearValues(colorCount + hasDepth);
    for (uint32 i = 0; i < colorCount; i++)
        clearValues[i] = VkClearValue {
            .color = {{clearColor.x, clearColor.y, clearColor.z, clearColor.w}} // (green-gray by default)
        };
    if (hasDepth)
        clearValues[colorCount] = VkClearValue {
            .depthStencil = {1.0f, 0}
        };    

    VkRenderPassBeginInfo renderPassInfo {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass      = renderPass->renderPass,
        .framebuffer     = renderPass->framebuffers[m_frameId % MAX_FRAME_COUNT],
        .renderArea      = {
            .offset = {0,0},
            .extent = {renderPass->dimensions.x, renderPass->dimensions.y}
        },
        .clearValueCount = (uint32)clearValues.size(),
        .pClearValues    = clearValues.data(),
    };
    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // prepare and return render pass renderer
    return m_passRenderer;
}

void CommandBuffer::endRenderPass(){
    vkCmdEndRenderPass(m_commandBuffer);
}

void CommandBuffer::submit(){
    std::vector<VkPipelineStageFlags> stageFlags;

    if (m_type == CommandType::TRANSFER) {
        stageFlags = {VK_PIPELINE_STAGE_TRANSFER_BIT};
    } else {
        stageFlags = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    }
    stageFlags = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

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


void CommandBuffer::waitFence(){
    VK_CHECK(vkWaitForFences(m_device->getDevice(), 1, &m_fence, VK_TRUE, UINT64_MAX));
}

void CommandBuffer::resetFence(){
    VK_CHECK(vkResetFences(m_device->getDevice(), 1, &m_fence));
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