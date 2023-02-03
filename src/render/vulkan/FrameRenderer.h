#pragma once

#include "VulkanRenderer.h"
#include "resource/VulkanResourceManager.h"

namespace spr::gfx {
class FrameRenderer {
public:
    FrameRenderer(VulkanResourceManager& rm, VkFormat displayFormat, glm::vec3 dimensions, Handle<Texture> result);
    ~FrameRenderer();

    RenderFrame& getCurrentFrame();
    void render();
    void destroy(VulkanResourceManager& rm);

private:
    Handle<RenderPassLayout> m_renderPassLayout;
    Handle<RenderPass> m_renderPass;
    Handle<Shader> m_shader;
    Handle<Texture> result;
};
}