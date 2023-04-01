#include "RenderPassRenderer.h"
#include "DescriptorSetHandler.h"
#include <vulkan/vulkan_core.h>


namespace spr::gfx {

RenderPassRenderer::RenderPassRenderer(){

}

RenderPassRenderer::RenderPassRenderer(VulkanResourceManager* rm, VkCommandBuffer commandBuffer, uint32 frameIndex){
    m_rm = rm;
    m_commandBuffer = commandBuffer;
    m_frameId = 0;
    m_frameIndex = frameIndex;

    m_descSetHandler = DescriptorSetHandler(rm, commandBuffer);
}

RenderPassRenderer::~RenderPassRenderer(){
    
}


void RenderPassRenderer::drawSubpass(PassContext context, std::vector<Batch>& batches){
    glm::uvec3 screenDim = m_rm->m_screenDim;

    // viewport
    VkViewport viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width  = (float)screenDim.x,
        .height = (float)screenDim.y,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    // scissor
    VkRect2D scissor{
        .offset = {0, 0},
        .extent = {screenDim.x, screenDim.y}
    };
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
    
    // bind the current pipeline
    Shader* shader = m_rm->get<Shader>(context.shader);
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline);

    // set and update descriptor bindings
    m_descSetHandler.set(0, context.set0);
    m_descSetHandler.set(1, context.set1);
    m_descSetHandler.set(2, context.set2);
    m_descSetHandler.set(3, context.set3);
    m_descSetHandler.updateBindings(shader->layout, m_frameIndex);
    
    // draw every batch:
    // here, a batch is a collection of mesh draws that
    // share a material (or subset of material flags)
    for (Batch& batch : batches){
        vkCmdDrawIndexed(m_commandBuffer, batch.indexCount, batch.drawCount, batch.firstIndex, 0, 0);
    }
}

void RenderPassRenderer::setFrameId(uint32 frameId){
    m_frameId = frameId;
    uint32 frameIndex = m_frameId % MAX_FRAME_COUNT;
    if (frameIndex != m_frameIndex){
        std::string errMsg("[RenderPassRenderer] Frame Index mismatch! ");
        std::string errInfo("Expected: " + std::to_string(m_frameIndex) + ", got: " + std::to_string(frameIndex));
        SprLog::error(errMsg + errInfo);
    }
}

}