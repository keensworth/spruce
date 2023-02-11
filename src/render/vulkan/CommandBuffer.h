#pragma once

#include <vulkan/vulkan.h>
#include <spruce_core.h>
#include <vulkan/vulkan_core.h>
#include "RenderPassRenderer.h"
#include "UploadHandler.h"
#include "../../debug/SprLog.h"
#include "resource/VulkanResourceManager.h"
#include "VulkanDevice.h"

namespace spr::gfx{

typedef enum CommandType : uint32 {
    TRANSFER = 0,
    OFFSCREEN = 1,
    MAIN = 2
} CommandType;


class CommandBuffer{
public:
    CommandBuffer();
    CommandBuffer(VulkanDevice device, CommandType commandType, VkCommandBuffer commandBuffer, VulkanResourceManager* rm, VkQueue queue);
    ~CommandBuffer();

    // rendering commands
    RenderPassRenderer& beginRenderPass(Handle<RenderPass> renderPass);
    void endRenderPass();

    // transfer commands
    UploadHandler& beginTransfer();

    // shared
    void submit();

    VkCommandBuffer getCommandBuffer();
    VkSemaphore getSemaphore();

    void setSemaphoreDependencies(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores);

private:
    CommandType m_type;
    VkCommandBuffer m_commandBuffer;
    VulkanResourceManager* m_rm;
    VkQueue m_queue;
    VkSemaphore m_semaphore;
    std::vector<VkSemaphore> m_waitSemaphores;
    std::vector<VkSemaphore> m_signalSemaphores;

    RenderPassRenderer m_passRenderer;
    UploadHandler m_uploadHandler;

    uint32 m_frame;
    void setFrame(uint32 frame);

    friend class CommandPool;
};
}