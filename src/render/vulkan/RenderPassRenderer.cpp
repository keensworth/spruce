#include "RenderPassRenderer.h"
#include "resource/ResourceTypes.h"
#include "resource/VulkanResourceManager.h"
#include "../../external/volk/volk.h"
#include "../scene/Draw.h"
#include "../../debug/SprLog.h"
#include <vulkan/vulkan_core.h>


namespace spr::gfx {

RenderPassRenderer::RenderPassRenderer(){

}

RenderPassRenderer::RenderPassRenderer(VulkanResourceManager* rm, VkCommandBuffer commandBuffer, uint32 frameIndex) : m_descSetHandler(rm, commandBuffer){
    m_rm = rm;
    m_commandBuffer = commandBuffer;
    m_frameId = 0;
    m_frameIndex = frameIndex;
}

RenderPassRenderer::~RenderPassRenderer(){
}

// draw batches of models+material that use the same pipeline
void RenderPassRenderer::drawSubpass(PassContext context, std::vector<Batch>& batches){
    // viewport
    VkViewport viewport {
        .x = 0.0f,
        .y = (float)m_dimensions.y,
        .width  = (float)m_dimensions.x,
        .height = -(float)m_dimensions.y, // avoid manual y-flip in shader
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    // scissor
    VkRect2D scissor{
        .offset = {0, 0},
        .extent = {m_dimensions.x, m_dimensions.y}
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
        vkCmdDrawIndexed(m_commandBuffer, batch.indexCount, batch.drawCount, batch.firstIndex, 0, batch.drawDataOffset);
    }
}

// draw batches of models+material that use the same pipeline, with specified vertexOffset
void RenderPassRenderer::drawSubpass(PassContext context, std::vector<Batch>& batches, uint32 vertexOffset){
    // viewport
    VkViewport viewport {
        .x = 0.0f,
        .y = (float)m_dimensions.y,
        .width  = (float)m_dimensions.x,
        .height = -(float)m_dimensions.y, // avoid manual y-flip in shader
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    // scissor
    VkRect2D scissor{
        .offset = {0, 0},
        .extent = {m_dimensions.x, m_dimensions.y}
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
        vkCmdDrawIndexed(m_commandBuffer, batch.indexCount, batch.drawCount, batch.firstIndex, vertexOffset, batch.drawDataOffset);
    }
}

// draw a single batch, with specified vertexOffset and firstInstance
void RenderPassRenderer::drawSubpass(PassContext context, Batch batch, uint32 vertexOffset, uint32 firstInstance){
    // viewport
    VkViewport viewport {
        .x = 0.0f,
        .y = (float)m_dimensions.y,
        .width  = (float)m_dimensions.x,
        .height = -(float)m_dimensions.y, // avoid manual y-flip in shader
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    // scissor
    VkRect2D scissor{
        .offset = {0, 0},
        .extent = {m_dimensions.x, m_dimensions.y}
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
    
    // draw the batch:
    vkCmdDrawIndexed(m_commandBuffer, batch.indexCount, batch.drawCount, batch.firstIndex, vertexOffset, firstInstance);
}

// dispatch compute pipeline
void RenderPassRenderer::dispatch(PassContext context, glm::uvec3 groupCount){  
    // bind the current pipeline
    Shader* shader = m_rm->get<Shader>(context.shader);
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, shader->pipeline);

    // set and update descriptor bindings
    m_descSetHandler.set(0, context.set0);
    m_descSetHandler.set(1, context.set1);
    m_descSetHandler.set(2, context.set2);
    m_descSetHandler.set(3, context.set3);
    m_descSetHandler.updateBindings(shader->layout, m_frameIndex);
    
    // draw the batch:
    vkCmdDispatch(m_commandBuffer, groupCount.x, groupCount.y, groupCount.z);
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

void RenderPassRenderer::setDimensions(glm::uvec3& dimensions){
    m_dimensions = dimensions;
}

}