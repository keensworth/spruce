#include "CommandBuffer.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

CommandBuffer::CommandBuffer(CommandType commandType, VkCommandBuffer commandBuffer, VulkanResourceManager* rm){
    m_type = commandType;
    m_commandBuffer = commandBuffer;
    m_rm = rm;
}

CommandBuffer::~CommandBuffer(){}

UploadHandler& CommandBuffer::beginTransfer(){
    if (m_type != CommandType::TRANSFER){
        SprLog::warn("CommandBuffer: Not a transfer command buffer");
    }

    return m_uploadHandler;
}

void CommandBuffer::endTransfer(){}

RenderPassRenderer& CommandBuffer::beginRenderPass(Handle<RenderPass> renderPassHandle){
    // make sure user is accessing correct commandbuffer
    if (m_type != CommandType::OFFSCREEN && m_type != CommandType::MAIN){
        SprLog::warn("CommandBuffer: Not a render command buffer");
    }

    // begin the renderpass
    RenderPass* renderPass = m_rm->get<RenderPass>(renderPassHandle);
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
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = renderPass->renderPass,
        .framebuffer = renderPass->framebuffer,
        .renderArea = {
            .offset = {0,0},
            .extent = {renderPass->dimensions.x, renderPass->dimensions.y}
        },
        .clearValueCount = (uint32)clearValues.size(),
        .pClearValues = clearValues.data(),
    };
    vkCmdBeginRenderPass(m_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // return the pass renderer
    return m_passRenderer;
}

void CommandBuffer::endRenderPass(){
    vkCmdEndRenderPass(m_commandBuffer);
}

void CommandBuffer::submit(){
    vkEndCommandBuffer(m_commandBuffer);

    vkQueueSubmit(queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence)
}

VkCommandBuffer CommandBuffer::getCommandBuffer(){
    return m_commandBuffer;
}

}