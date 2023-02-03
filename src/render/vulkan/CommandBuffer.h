#pragma once

#include <vulkan/vulkan.h>
#include <spruce_core.h>
#include <vulkan/vulkan_core.h>
#include "RenderPassRenderer.h"
#include "UploadHandler.h"
#include "../../debug/SprLog.h"
#include "resource/VulkanResourceManager.h"

namespace spr::gfx{

typedef enum CommandType : uint32 {
    TRANSFER = 0,
    OFFSCREEN = 1,
    MAIN = 2
} CommandType;


class CommandBuffer{
public:
    CommandBuffer(CommandType commandType, VkCommandBuffer commandBuffer, VulkanResourceManager* rm);
    ~CommandBuffer();

    UploadHandler& beginTransfer();
    void endTransfer();
    RenderPassRenderer& beginRenderPass(Handle<RenderPass> renderPass);
    void endRenderPass();
    void submit();

    VkCommandBuffer getCommandBuffer();

private:
    CommandType m_type;
    VkCommandBuffer m_commandBuffer;
    VulkanResourceManager* m_rm;

    RenderPassRenderer m_passRenderer;
    UploadHandler m_uploadHandler;
};
}