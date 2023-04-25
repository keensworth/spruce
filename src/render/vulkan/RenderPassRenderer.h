#pragma once

#include <vector>
#include "../../external/volk/volk.h"
#include "DescriptorSetHandler.h"

namespace spr::gfx{

class VulkanResourceManager;
struct Shader;
struct Batch;

typedef struct PassContext {
    Handle<Shader> shader;
    Handle<DescriptorSet> set0;
    Handle<DescriptorSet> set1;
    Handle<DescriptorSet> set2;
    Handle<DescriptorSet> set3;
} PassContext;


class RenderPassRenderer{
public:
    RenderPassRenderer();
    RenderPassRenderer(VulkanResourceManager* rm, VkCommandBuffer commandBuffer, uint32 frameIndex);
    ~RenderPassRenderer();

    void drawSubpass(PassContext context, std::vector<Batch>& batches);
    void drawSubpass(PassContext context, Batch batch, uint32 vertexOffset);
    void setFrameId(uint32 frameId);

private: // non-owning
    VulkanResourceManager* m_rm;
    VkCommandBuffer m_commandBuffer;
    DescriptorSetHandler m_descSetHandler;

    uint32 m_frameId;
    uint32 m_frameIndex;
};
}