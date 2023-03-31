#include "SceneManager.h"
#include "scene/Mesh.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include "vulkan/vulkan_core.h"
#include <cctype>

namespace spr::gfx {

SceneManager::SceneManager(){}

SceneManager::SceneManager(VulkanResourceManager& rm){  
    m_rm = &rm;

    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_batchManagers[i] = BatchManager();
    }

    // per frame resource temp buffers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){        
        m_transforms[i] = TempBuffer<Transform>(MAX_DRAWS);
        m_drawData[i] = TempBuffer<DrawData>(MAX_DRAWS);
        m_lights[i] = TempBuffer<Light>(MAX_LIGHTS);
        m_cameras[i] = TempBuffer<Camera>(1);
        m_sceneData[i] = TempBuffer<Scene>(1);
    }
}

SceneManager::~SceneManager(){
    if (m_destroyed)
        return;

    SprLog::warn("[SceneManager] [~] Calling destroy() in destructor");
    destroy();
}


void SceneManager::initializeAssets(SprResourceManager &rm){
    // TODO
    //

    // initialize buffers
    PrimitiveCounts counts = {
        .vertexCount   = 0,
        .indexCount    = 0,
        .materialCount = 0,
        .textureCount  = 0
    };
    initBuffers(counts);
}


void SceneManager::insertMesh(uint32 frame, uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose){
    // get mesh data and fill draw
    MeshInfo& meshInfo = m_meshInfo[meshId];
    DrawData draw = {
        .vertexOffset   = meshInfo.vertexOffset,  
        .materialIndex  = meshInfo.materialIndex, 
        .transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert({
            model,
            modelInvTranspose
        })
    };

    // fill out batch info, which will either initialize
    // a new batch or update an existing one
    Batch batchInfo = {
        .meshId         = meshId,
        .materialFlags  = materialFlags,
        .indexCount     = meshInfo.indexCount,
        .firstIndex     = meshInfo.firstIndex,
        .drawDataOffset = 0,
        .drawCount      = 1
    };

    m_batchManagers[frame % MAX_FRAME_COUNT].addDraw(draw, batchInfo);
}


void SceneManager::insertLight(uint32 frame, Light light){
    m_lights[frame % MAX_FRAME_COUNT].insert(light);
}


void SceneManager::updateCamera(uint32 frame, Camera camera){
    m_cameras[frame % MAX_FRAME_COUNT].insert(camera);
}


BatchManager& SceneManager::getBatchManager(uint32 frame) {
    return m_batchManagers[frame % MAX_FRAME_COUNT];
}


void SceneManager::initBuffers(PrimitiveCounts counts){
    // per frame resource handles
    Handle<Buffer> m_lightsBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_LIGHTS * MAX_FRAME_COUNT * sizeof(Light)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_transformBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_DRAWS * MAX_FRAME_COUNT * sizeof(Transform)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_drawDataBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_DRAWS * MAX_FRAME_COUNT * sizeof(DrawData)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_cameraBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_FRAME_COUNT * sizeof(Camera)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_sceneBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_FRAME_COUNT * sizeof(Scene)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    // global resource handles
    Handle<Buffer> m_positionsBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (counts.vertexCount * sizeof(VertexPosition)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_attributesBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (counts.vertexCount * sizeof(VertexAttributes)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_indexBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (counts.indexCount * sizeof(uint32)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_materialsBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (counts.materialCount * sizeof(MaterialData)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_textureDescBuffer = m_rm->create<Buffer>(BufferDesc{
        .byteSize = (uint32) (counts.textureCount * sizeof(uint32)), // TODO
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });
}

void SceneManager::reset(uint32 frame) {
    m_batchManagers[frame % MAX_FRAME_COUNT].reset();
    m_cameras[frame % MAX_FRAME_COUNT].reset();
    m_transforms[frame % MAX_FRAME_COUNT].reset();
    m_lights[frame % MAX_FRAME_COUNT].reset();
    m_drawData[frame % MAX_FRAME_COUNT].reset();
    m_sceneData[frame % MAX_FRAME_COUNT].reset();
}

void SceneManager::destroy(){
    // destroy per-frame batch managers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_batchManagers[i].destroy();
    }

    // destroy per-frame tempbuffers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_drawData[i].reset();
        m_cameras[i].reset();
        m_sceneData[i].reset();
        m_lights[i].reset();
        m_transforms[i].reset();
    }

    // destroy per-frame resources
    m_rm->remove(m_lightsBuffer);
    m_rm->remove(m_transformBuffer);
    m_rm->remove(m_drawDataBuffer);
    m_rm->remove(m_cameraBuffer);
    m_rm->remove(m_sceneBuffer);

    // destroy global resources
    m_rm->remove(m_positionsBuffer);
    m_rm->remove(m_attributesBuffer);
    m_rm->remove(m_indexBuffer);
    m_rm->remove(m_materialsBuffer);
    m_rm->remove(m_textureDescBuffer);

    m_destroyed = true;
}

}