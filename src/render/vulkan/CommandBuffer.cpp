#include "CommandBuffer.h"

#include "UploadHandler.h"
#include "vulkan_core.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

CommandBuffer::CommandBuffer() {}

CommandBuffer::CommandBuffer(VulkanDevice& device, CommandType commandType, VkCommandBuffer commandBuffer, VulkanResourceManager* rm, VkQueue queue){
    m_type = commandType;
    m_commandBuffer = commandBuffer;
    m_rm = rm;
    m_device = &device;
    m_queue = queue;
    m_frame = 0;

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
}

CommandBuffer::~CommandBuffer(){}

void CommandBuffer::begin(){
    VkCommandBufferBeginInfo beginInfo {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    };
    VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &beginInfo));
}

void CommandBuffer::end(){
    VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
}

RenderPassRenderer& CommandBuffer::beginRenderPass(Handle<RenderPass> handle){
    // make sure user is accessing correct commandbuffer
    if (m_type != CommandType::OFFSCREEN && m_type != CommandType::MAIN){
        SprLog::warn("CommandBuffer: Not a render command buffer");
    }

    // begin the renderpass
    RenderPass* renderPass = m_rm->get<RenderPass>(handle);
    std::vector<VkClearValue> clearValues(renderPass->hasDepthAttachment + renderPass->colorAttachments.size());
    for (uint32 i = 0; i < clearValues.size(); i++){
        if (i < clearValues.size() - 1){
            clearValues[i] = VkClearValue {
                .color = {{0.0f, 0.0f, 0.0f, 1.0f}}
            };
        } else {
            clearValues[i] = VkClearValue {
                .depthStencil = {1.0f, 0}
            };
        }
    }
    VkRenderPassBeginInfo renderPassInfo {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass      = renderPass->renderPass,
        .framebuffer     = renderPass->framebuffers[m_frame % MAX_FRAME_COUNT],
        .renderArea      = {
            .offset = {0,0},
            .extent = {renderPass->dimensions.x, renderPass->dimensions.y}
        },
        .clearValueCount = (uint32)clearValues.size(),
        .pClearValues    = clearValues.data(),
    };
    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // return the pass renderer
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

void CommandBuffer::resetFence(){
    VK_CHECK(vkResetFences(m_device->getDevice(), 1, &m_fence));
}

void CommandBuffer::setSemaphoreDependencies(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores){
    m_waitSemaphores = waitSemaphores;
    m_signalSemaphores = signalSemaphores;
}

void CommandBuffer::setFrame(uint32 frame) {
    m_frame = frame;
}

}