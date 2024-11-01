#pragma once

#include <spruce_core.h>
#include "../../external/volk/volk.h"
#include "RenderPassRenderer.h"

namespace spr::gfx{

class VulkanDevice;
class VulkanResourceManager;
struct RenderPass;
struct Framebuffer;
struct Buffer;

typedef enum CommandType : uint32 {
    TRANSFER = 0,
    OFFSCREEN = 1,
    MAIN = 2
} CommandType;

class CommandBuffer{
public:
    CommandBuffer();
    ~CommandBuffer();

    RenderPassRenderer& beginRenderPass(Handle<RenderPass> renderPass, glm::vec4 clearColor = glm::vec4{0.45098f, 0.52549f, 0.47058f, 1.0f});
    RenderPassRenderer& beginRenderPass(Handle<RenderPass> renderPass, Handle<Framebuffer> framebuffer, glm::vec4 clearColor = glm::vec4{0.45098f, 0.52549f, 0.47058f, 1.0f});
    void endRenderPass();

    RenderPassRenderer& beginComputePass();
    void endComputePass();
    
    void bindIndexBuffer(Handle<Buffer> indexBuffer);

    void submit();

    VkCommandBuffer getCommandBuffer();
    VkSemaphore getSemaphore();
    VkFence getFence();
    void waitFence();
    void resetFence();

    bool isRecording();
    
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

    bool m_initialized = false;
    bool m_destroyed = false;
    bool m_recording = false;
    bool m_fenceInUse = false;

    void setFrameId(uint32 frameId);
    void begin();
    void end();
    void init(VulkanDevice& device, VulkanResourceManager* rm, uint32 frameIndex, CommandType commandType, VkCommandBuffer commandBuffer, VkQueue queue);
    void destroy();

    friend class VulkanRenderer;
    friend class UploadHandler;
    friend class CommandPool;
};
}