#pragma once

#include "spruce_core.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include "vulkan/VulkanRenderer.h"
#include "SceneManager.h"
#include "RenderCoordinator.h"
#include "../core/util/Span.h"


namespace spr {
    class Window;
}

namespace spr::gfx {
    
struct Transform;
struct Light;
struct Camera;

struct TransformInfo {
    glm::vec3 position {0.f, 0.f, 0.f};
    glm::quat rotation {1.f, 0.f, 0.f, 0.f};
    float scale = 1.f;
};

class SprRenderer {
public:
    SprRenderer(Window* window);
    ~SprRenderer();
    void loadAssets(SprResourceManager& rm);

    // render accumulated scene
    void render();

    // set scene camera
    void updateCamera(const Camera& camera);

    // add lights to scene
    void insertLight(const Light& light);
    void insertLights(Span<const Light> lights);

    // ╔══════════════════════════════════════════════════════════════════════════╗
    // ║     Models                                                               ║
    // ╚══════════════════════════════════════════════════════════════════════════╝

    void insertModel(uint32 modelId, const TransformInfo& transformInfo);
    void insertModel(uint32 modelId, uint32 materialFlags, const TransformInfo& transformInfo);
    void insertModel(uint32 modelId, const Transform& transform);
    void insertModel(uint32 modelId, uint32 materialFlags, const Transform& transform);


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
    void insertMesh(        uint32 meshId,       const Transform& transform);
    // batch meshes, shared transform
    void insertMeshes(Span<uint32> meshIds,      const Transform& transform);
    // batch meshes
    void insertMeshes(Span<uint32> meshIds, Span<const Transform> transforms);

    // single mesh
    void insertMesh(        uint32 meshId,        uint32 materialFlags,       const Transform& transform);
    // batch meshes, shared material, shared transform
    void insertMeshes(Span<uint32> meshIds,       uint32 materialFlags,       const Transform& transform);
    // batch meshes, shared material
    void insertMeshes(Span<uint32> meshIds,       uint32 materialFlags,  Span<const Transform> transforms);
    // batch meshes, shared transform
    void insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags,      const Transform& transform);
    // batch meshes
    void insertMeshes(Span<uint32> meshIds, Span<uint32> materialsFlags, Span<const Transform> transforms);

    Transform buildTransform(const TransformInfo& info);
    
private:
    VulkanRenderer m_renderer;
    VulkanResourceManager m_rm;
        
    SceneManager m_sceneManager;
    RenderCoordinator m_renderCoordinator;
    uint32 m_frameId;    

    // non-owning
    Window* m_window;
    SprResourceManager* m_srm;
};
}