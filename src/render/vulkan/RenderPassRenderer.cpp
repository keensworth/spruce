#include "RenderPassRenderer.h"
#include <vulkan/vulkan_core.h>


namespace spr::gfx {

RenderPassRenderer::RenderPassRenderer(){

}

RenderPassRenderer::RenderPassRenderer(VulkanResourceManager* rm, VkCommandBuffer commandBuffer, glm::uvec3 dimensions, uint32 frameIndex){
    m_rm = rm;
    m_commandBuffer = commandBuffer;
    m_dim = dimensions;
    m_frameIndex = frameIndex;
}

RenderPassRenderer::~RenderPassRenderer(){
    
}


void RenderPassRenderer::drawSubpass(PassContext context, std::vector<Batch>& batches){
    VkViewport viewport {
        .x = 0.0f,
        .y = 0.0f,
        .width  = (float)m_dim.x,
        .height = (float)m_dim.y,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(m_commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = {m_dim.x, m_dim.y}
    };
    vkCmdSetScissor(m_commandBuffer, 0, 1, &scissor);
    
    Shader* shader = m_rm->get<Shader>(context.shader);
    vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->pipeline);

    m_descSetHandler.set(0, context.set0);
    m_descSetHandler.set(1, context.set1);
    m_descSetHandler.set(2, context.set2);
    m_descSetHandler.set(3, context.set3);
    m_descSetHandler.updateBindings(shader->layout, m_frameIndex);
    
    for (Batch& batch : batches){
        vkCmdDrawIndexed(m_commandBuffer, batch.indexCount, batch.drawCount, batch.firstIndex, 0, 0);
    }
}

}