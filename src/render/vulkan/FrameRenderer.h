#pragma once

#include "VulkanRenderer.h"
#include "resource/ResourceTypes.h"
#include "resource/VulkanResourceManager.h"

namespace spr::gfx {
class FrameRenderer {
public:
    FrameRenderer();
    FrameRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, glm::uvec3 dimensions);
    ~FrameRenderer();


    void init(std::vector<VkImageView>& swapchainImages, Handle<TextureAttachment> input);
    void render();
    void destroy();

private:
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;
    glm::uvec3 m_dim;

    Handle<RenderPassLayout> m_renderPassLayout;
    Handle<RenderPass> m_renderPass;
    Handle<DescriptorSetLayout> m_descriptorSetLayout;
    Handle<DescriptorSet> m_descriptorSet;
    Handle<Shader> m_shader;
};
}