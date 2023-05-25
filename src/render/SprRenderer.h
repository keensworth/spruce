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
    void insertLight(const gfx::Light& light);
    void insertLights(Span<const gfx::Light> lights);

    // ╔══════════════════════════════════════════════════════════════════════════╗
    // ║     Models                                                               ║
    // ╚══════════════════════════════════════════════════════════════════════════╝

    void insertModel(uint32 modelId, const TransformInfo& transformInfo);
    void insertModel(uint32 modelId, uint32 materialFlags, const TransformInfo& transformInfo);
    void insertModel(uint32 modelId, const gfx::Transform& transform);
    void insertModel(uint32 modelId, uint32 materialFlags, const gfx::Transform& transform);


    // ╔══════════════════════════════════════════════════════════════════════════╗
    // ║     Meshes                                                               ║
    // ╚══════════════════════════════════════════════════════════════════════════╝

    // single mesh
    void insertMesh(         uint32 meshId,       const TransformInfo& transformInfo);
    // batch meshes, shared transform
    void insertMeshes(Span<uint32> meshIds,       const TransformInfo& transformInfo);
    // batch meshes
    void insertMeshes(Span<uint32> meshIds,  Span<const TransformInfo> transformInfos);

    // single mesh
    void insertMesh(        uint32 meshId,        uint32 materialFlags,       const TransformInfo& transformInfo);
    // batch meshes, shared material, shared transform
    void insertMeshes(Span<uint32> meshIds,       uint32 materialFlags,       const TransformInfo& transformInfo);
    // batch meshes, shared material
    void insertMeshes(Span<uint32> meshIds,       uint32 materialFlags,  Span<const TransformInfo> transformInfos);
    // batch meshes, shared transform
    void insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags,      const TransformInfo& transformInfo);
    // batch meshes
    void insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags, Span<const TransformInfo> transformInfos);
    

    // single mesh
    void insertMesh(        uint32 meshId,       const gfx::Transform& transform);
    // batch meshes, shared transform
    void insertMeshes(Span<uint32> meshIds,      const gfx::Transform& transform);
    // batch meshes
    void insertMeshes(Span<uint32> meshIds, Span<const gfx::Transform> transforms);

    // single mesh
    void insertMesh(        uint32 meshId,        uint32 materialFlags,       const gfx::Transform& transform);
    // batch meshes, shared material, shared transform
    void insertMeshes(Span<uint32> meshIds,       uint32 materialFlags,       const gfx::Transform& transform);
    // batch meshes, shared material
    void insertMeshes(Span<uint32> meshIds,       uint32 materialFlags,  Span<const gfx::Transform> transforms);
    // batch meshes, shared transform
    void insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags,      const gfx::Transform& transform);
    // batch meshes
    void insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags, Span<const gfx::Transform> transforms);

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