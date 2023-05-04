#include "DescriptorSetHandler.h"
#include "../../external/volk/volk.h"
#include "resource/ResourceTypes.h"
#include "../scene/Draw.h"
#include "resource/VulkanResourceManager.h"
#include "../../debug/SprLog.h"

namespace spr::gfx {

DescriptorSetHandler::DescriptorSetHandler(){

}

DescriptorSetHandler::DescriptorSetHandler(VulkanResourceManager* rm, VkCommandBuffer commandBuffer){
    m_rm = rm;
    m_commandBuffer = commandBuffer;
    m_initialized = true;

    reset();
}

DescriptorSetHandler::~DescriptorSetHandler(){
    if (m_initialized)
        SprLog::info("[DescriptorSetHandler] [destroy] destroyed...");
}

void DescriptorSetHandler::set(uint32 set, Handle<DescriptorSet> handle){
    if (set >= 4){
        SprLog::warn("[DescriptorSetHandler] Set does not exist, set must be < 4");
        return;
    }
    
    if (handle != m_sets[set])
        m_sets[set] = handle;
}

void DescriptorSetHandler::updateBindings(VkPipelineLayout pipelineLayout, uint32 frameIndex){
    for (uint32 i = 0; i < 4; i++){
        if (m_sets[i].isValid()){
            DescriptorSet* descSet = m_rm->get<DescriptorSet>(m_sets[i]);
            VkDescriptorSet set = descSet->global ? descSet->descriptorSets[0] : descSet->descriptorSets[frameIndex];
            vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, i, 1, &set, 0, NULL);
        }
    }
}

void DescriptorSetHandler::reset(){
    m_sets[0] = Handle<DescriptorSet>();
    m_sets[1] = Handle<DescriptorSet>();
    m_sets[2] = Handle<DescriptorSet>();
    m_sets[3] = Handle<DescriptorSet>();
}

}