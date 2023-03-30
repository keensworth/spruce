#pragma once

#include <vulkan/vulkan.h>
#include <spruce_core.h>
#include <vulkan/vulkan_core.h>
#include "RenderPassRenderer.h"
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
    CommandBuffer(VulkanDevice& device, CommandType commandType, VkCommandBuffer commandBuffer, VulkanResourceManager* rm, VkQueue queue, uint32 frameIndex);
    ~CommandBuffer();

    RenderPassRenderer& beginRenderPass(Handle<RenderPass> renderPass, glm::vec4 clearColor);
    void endRenderPass();
    void submit();

    VkCommandBuffer getCommandBuffer();
    VkSemaphore getSemaphore();
    VkFence getFence();
    void waitFence();
    void resetFence();

    void setSemaphoreDependencies(std::vector<VkSemaphore> waitSemaphores, std::vector<VkSemaphore> signalSemaphores);


private: // owning
    VkFence m_fence;
    VkSemaphore m_semaphore;

private: // non-owning
    CommandType m_type;
    VkCommandBuffer m_commandBuffer;
    std::vector<VkSemaphore> m_waitSemaphores;
    std::vector<VkSemaphore> m_signalSemaphores;
    VkQueue m_queue;
    
    VulkanDevice* m_device; 
    VulkanResourceManager* m_rm;
    RenderPassRenderer m_passRenderer;

    uint32 m_frameId;
    uint32 m_frameIndex;

    bool m_destroyed = false;

    void setFrameId(uint32 frameId);
    void begin();
    void end();
    void destroy();

    friend class VulkanRenderer;
    friend class UploadHandler;
    friend class CommandPool;
};
}