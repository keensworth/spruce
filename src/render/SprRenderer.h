#pragma once

#include "spruce_core.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include "vulkan/VulkanRenderer.h"
#include "SceneManager.h"
#include "RenderCoordinator.h"
#include "../core/util/Span.h"


namespace spr {

class SprWindow;

namespace gfx {
    struct Transform;
    struct Light;
    struct Camera;
}

struct TransformInfo {
    glm::vec3 position {0.f, 0.f, 0.f};
    glm::quat rotation {1.f, 0.f, 0.f, 0.f};
    float scale = 1.f;
};

class SprRenderer {
public:
    SprRenderer(SprWindow* window);
    ~SprRenderer();
    void loadAssets(SprResourceManager& rm);

    // render accumulated scene
    void render();

    // set scene camera
    void updateCamera(const gfx::Camera& camera);

    // add lights to scene
    void insertLight(uint32 id, const gfx::Light& light);
    void insertLights(Span<uint32> ids, Span<const gfx::Light> lights);

    void updateLight(uint32 id, const gfx::Light& light);

    // ╔══════════════════════════════════════════════════════════════════════════╗
    // ║     Models                                                               ║
    // ╚══════════════════════════════════════════════════════════════════════════╝

    // insert model w/ transform (initial)
    void insertModel(uint32 id, uint32 modelId, const TransformInfo& transformInfo);
    void insertModel(uint32 id, uint32 modelId, uint32 materialFlags, const TransformInfo& transformInfo);
    void insertModel(uint32 id, uint32 modelId, const gfx::Transform& transform);
    void insertModel(uint32 id, uint32 modelId, uint32 materialFlags, const gfx::Transform& transform);

    // insert model (recurring)
    void insertModel(uint32 id, uint32 modelId);
    void insertModel(uint32 id, uint32 modelId, uint32 materialFlags);

    // update existing model's transform
    void updateModel(uint32 id, uint32 modelId, const TransformInfo& transformInfo);
    void updateModel(uint32 id, uint32 modelId, const gfx::Transform& transform);

    gfx::Transform buildTransform(const TransformInfo& info);
    
private:
    gfx::VulkanRenderer m_renderer;
    gfx::VulkanResourceManager m_rm;
        
    gfx::SceneManager m_sceneManager;
    gfx::RenderCoordinator m_renderCoordinator;
    uint32 m_frameId;    

    // non-owning
    SprWindow* m_window;
    SprResourceManager* m_srm;
};
}