#include "SceneManager.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "scene/GfxAssetLoader.h"
#include "scene/Material.h"
#include "scene/Mesh.h"
#include "vulkan/gfx_vulkan_core.h"
#include "vulkan/resource/ResourceFlags.h"
#include "vulkan/resource/ResourceTypes.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include "vulkan/UploadHandler.h"
#include "../resource/SprResourceManager.h"
#include <cctype>
#include "scene/SceneData.h"
#include "../debug/SprLog.h"

namespace spr::gfx {

SceneManager::SceneManager(){}

SceneManager::~SceneManager(){
    if (m_destroyed)
        return;

    SprLog::warn("[SceneManager] [~] Calling destroy() in destructor");
    destroy();
}

void SceneManager::init(VulkanResourceManager& rm){
    m_rm = &rm;

    // per frame resource temp buffers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){        
        m_transforms[i] = TempBuffer<Transform>(MAX_DRAWS);
        m_drawData[i] = TempBuffer<DrawData>(MAX_DRAWS);
        m_lights[i] = TempBuffer<Light>(MAX_LIGHTS);
        m_cameras[i] = TempBuffer<Camera>(1);
        m_sceneData[i] = TempBuffer<Scene>(1);
        m_sceneData[i].insert({});
    }
}


void SceneManager::insertMeshes(uint32 frame, Span<uint32> meshIds, Span<uint32> materialsFlags, Span<const Transform> transforms){
    bool sharedMaterial = (meshIds.size() != materialsFlags.size() && materialsFlags.size() == 1);

    for (uint32 i = 0; i < meshIds.size(); i++){
        // get mesh data and fill draw
        MeshInfo& meshInfo = m_meshInfo[meshIds[i]];
        DrawData draw = {
            .vertexOffset   = meshInfo.vertexOffset,  
            .materialIndex  = meshInfo.materialIndex, 
            .transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert(transforms[i])
        };

        // fill out batch info, which will either initialize
        // a new batch or update an existing one
        Batch batchInfo = {
            .meshId         = meshIds[i],
            .materialFlags  = !sharedMaterial ? materialsFlags[i] : materialsFlags[0],
            .indexCount     = meshInfo.indexCount,
            .firstIndex     = meshInfo.firstIndex,
            .drawDataOffset = 0,
            .drawCount      = 1
        };

        m_batchManagers[frame % MAX_FRAME_COUNT].addDraw(draw, batchInfo);
    }
}

void SceneManager::insertMeshes(uint32 frame, Span<uint32> meshIds, Span<uint32> materialsFlags, const Transform& transform ){
    bool sharedMaterial = (meshIds.size() != materialsFlags.size() && materialsFlags.size() == 1);

    uint32 transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert(transform);

    for (uint32 i = 0; i < meshIds.size(); i++){
        // get mesh data and fill draw
        MeshInfo& meshInfo = m_meshInfo[meshIds[i]];
        DrawData draw = {
            .vertexOffset   = meshInfo.vertexOffset,  
            .materialIndex  = meshInfo.materialIndex, 
            .transformIndex = transformIndex
        };

        // fill out batch info, which will either initialize
        // a new batch or update an existing one
        Batch batchInfo = {
            .meshId         = meshIds[i],
            .materialFlags  = !sharedMaterial ? materialsFlags[i] : materialsFlags[0],
            .indexCount     = meshInfo.indexCount,
            .firstIndex     = meshInfo.firstIndex,
            .drawDataOffset = 0,
            .drawCount      = 1
        };

        m_batchManagers[frame % MAX_FRAME_COUNT].addDraw(draw, batchInfo);
    }
}

void SceneManager::insertMeshes(uint32 frame, Span<uint32> meshIds, uint32 materialFlags , Span<const Transform> transforms){
    for (uint32 i = 0; i < meshIds.size(); i++){
        // get mesh data and fill draw
        MeshInfo& meshInfo = m_meshInfo[meshIds[i]];
        DrawData draw = {
            .vertexOffset   = meshInfo.vertexOffset,  
            .materialIndex  = meshInfo.materialIndex, 
            .transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert(transforms[i])
        };

        // fill out batch info, which will either initialize
        // a new batch or update an existing one
        Batch batchInfo = {
            .meshId         = meshIds[i],
            .materialFlags  = materialFlags,
            .indexCount     = meshInfo.indexCount,
            .firstIndex     = meshInfo.firstIndex,
            .drawDataOffset = 0,
            .drawCount      = 1
        };

        m_batchManagers[frame % MAX_FRAME_COUNT].addDraw(draw, batchInfo);
    }
}

void SceneManager::insertMeshes(uint32 frame, Span<uint32> meshIds, uint32 materialFlags , const Transform& transform){
    uint32 transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert(transform);

    for (uint32 i = 0; i < meshIds.size(); i++){
        // get mesh data and fill draw
        MeshInfo& meshInfo = m_meshInfo[meshIds[i]];
        DrawData draw = {
            .vertexOffset   = meshInfo.vertexOffset,  
            .materialIndex  = meshInfo.materialIndex, 
            .transformIndex = transformIndex
        };

        // fill out batch info, which will either initialize
        // a new batch or update an existing one
        Batch batchInfo = {
            .meshId         = meshIds[i],
            .materialFlags  = materialFlags,
            .indexCount     = meshInfo.indexCount,
            .firstIndex     = meshInfo.firstIndex,
            .drawDataOffset = 0,
            .drawCount      = 1
        };

        m_batchManagers[frame % MAX_FRAME_COUNT].addDraw(draw, batchInfo);
    }
}


void SceneManager::insertLights(uint32 frame, Span<const Light> lights){
    m_lights[frame % MAX_FRAME_COUNT].insert(lights.data(), lights.size());
    m_sceneData[frame % MAX_FRAME_COUNT].getData()[0].lightCount += lights.size();
}


void SceneManager::updateCamera(uint32 frame, glm::vec2 screenDim, const Camera& camera){
    m_cameras[frame % MAX_FRAME_COUNT].insert(camera);

    // pre-compute viewProjection matrix
    glm::mat4 view = glm::lookAt(camera.pos, camera.pos + camera.dir, camera.up);
    glm::mat4 proj = glm::perspective(camera.fov, (screenDim.x/screenDim.y), camera.near, camera.far);
    m_sceneData[frame % MAX_FRAME_COUNT][0].viewProj = {proj * view};
}


void SceneManager::uploadGlobalResources(UploadHandler& uploadHandler){
    uploadHandler.uploadBuffer(m_assetLoader.getVertexPositionData(), m_positionsBuffer);
    uploadHandler.uploadBuffer(m_assetLoader.getVertexAttributeData(), m_attributesBuffer);
    uploadHandler.uploadBuffer(m_assetLoader.getMaterialData(), m_materialsBuffer);
    uploadHandler.uploadBuffer(m_assetLoader.getVertexIndicesData(), m_indexBuffer);

    std::vector<TextureInfo>& textures = m_assetLoader.getTextureData();
    for (uint32 i = 0; i < m_assetLoader.getPrimitiveCounts().textureCount; i++){
        uploadHandler.uploadTexture(textures[i].data, m_textures[i]);
    }
}

void SceneManager::uploadPerFrameResources(UploadHandler& uploadHandler, uint32 frame){
    uploadHandler.uploadDyanmicBuffer(m_sceneData[frame % MAX_FRAME_COUNT], m_sceneBuffer);
    uploadHandler.uploadDyanmicBuffer(m_cameras[frame % MAX_FRAME_COUNT], m_cameraBuffer);
    uploadHandler.uploadDyanmicBuffer(m_lights[frame % MAX_FRAME_COUNT], m_lightsBuffer);
    uploadHandler.uploadDyanmicBuffer(m_transforms[frame % MAX_FRAME_COUNT], m_transformBuffer);
    
    m_batchManagers[frame % MAX_FRAME_COUNT].getDrawData(m_drawData[frame % MAX_FRAME_COUNT]);
    uploadHandler.uploadDyanmicBuffer(m_drawData[frame % MAX_FRAME_COUNT], m_drawDataBuffer);
}


Handle<DescriptorSet> SceneManager::getGlobalDescriptorSet(){
    return m_globalDescriptorSet;
}

Handle<DescriptorSetLayout> SceneManager::getGlobalDescriptorSetLayout(){
    return m_globalDescriptorSetLayout;
}

Handle<DescriptorSet> SceneManager::getPerFrameDescriptorSet(uint32 frame){
    return m_frameDescriptorSets[frame % MAX_FRAME_COUNT];
}

Handle<DescriptorSet>* SceneManager::getPerFrameDescriptorSets(){
    return m_frameDescriptorSets;
}

Handle<DescriptorSetLayout> SceneManager::getPerFrameDescriptorSetLayout(){
    return m_frameDescriptorSetLayout;
}

Handle<Buffer> SceneManager::getIndexBuffer(){
    return m_indexBuffer;
}

BatchManager& SceneManager::getBatchManager(uint32 frame) {
    return m_batchManagers[frame % MAX_FRAME_COUNT];
}


void SceneManager::initializeAssets(SprResourceManager &rm, VulkanDevice* device){
    m_meshInfo = m_assetLoader.loadAssets(rm);

    PrimitiveCounts counts = m_assetLoader.getPrimitiveCounts();

    initTextures(counts.textureCount);
    initBuffers(counts, device);
    initDescriptorSets(device);
    
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_batchManagers[i].setQuadInfo({
            .indexCount = m_meshInfo[1].indexCount,
            .firstIndex = m_meshInfo[1].firstIndex,
            .drawCount = 1
        }, m_meshInfo[1].vertexOffset);
    }
}

void SceneManager::initBuffers(PrimitiveCounts counts, VulkanDevice* device){
    // per frame resource handles
    m_lightsBuffer = m_rm->create<Buffer>({
        .byteSize = (uint32) (MAX_LIGHTS * MAX_FRAME_COUNT * sizeof(Light)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    m_transformBuffer = m_rm->create<Buffer>({
        .byteSize = (uint32) (MAX_DRAWS * MAX_FRAME_COUNT * sizeof(Transform)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    m_drawDataBuffer = m_rm->create<Buffer>({
        .byteSize = (uint32) (MAX_DRAWS * MAX_FRAME_COUNT * sizeof(DrawData)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    m_cameraBuffer = m_rm->create<Buffer>({
        .byteSize = (uint32) (MAX_FRAME_COUNT * m_rm->alignedSize(sizeof(Camera))),
        .usage = Flags::BufferUsage::BU_UNIFORM_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    m_sceneBuffer = m_rm->create<Buffer>({
        .byteSize = (uint32) (MAX_FRAME_COUNT * m_rm->alignedSize(sizeof(Scene))),
        .usage = Flags::BufferUsage::BU_UNIFORM_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    // global resource handles
    m_positionsBuffer = m_rm->create<Buffer>({
        .byteSize = (uint32) (counts.vertexCount * sizeof(VertexPosition)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE
    });

    m_attributesBuffer = m_rm->create<Buffer>({
        .byteSize = (uint32) (counts.vertexCount * sizeof(VertexAttributes)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE
    });
    
    m_indexBuffer = m_rm->create<Buffer>({
        .byteSize = (uint32) (counts.indexCount * sizeof(uint16)),
        .usage = Flags::BufferUsage::BU_INDEX_BUFFER   |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE
    });

    m_materialsBuffer = m_rm->create<Buffer>({
        .byteSize = (uint32) (counts.materialCount * sizeof(MaterialData)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE
    });
}

void SceneManager::initTextures(uint32 textureCount){
    m_textures.resize(textureCount);
    
    std::vector<TextureInfo>& textureData = m_assetLoader.getTextureData();

    for (uint32 i = 0; i < textureCount; i++){
        TextureInfo& info = textureData[i];

        Flags::Format format;
        if (info.components == 1){
            format = info.srgb ? Flags::Format::R8_SRGB
                               : Flags::Format::R8_UNORM;
        } else if (info.components == 2){
            format = info.srgb ? Flags::Format::RG8_SRGB
                               : Flags::Format::RG8_UNORM;
        } else if (info.components == 3){
            format = info.srgb ? Flags::Format::RGB8_SRGB
                               : Flags::Format::RGB8_UNORM;
        } else {
            format = info.srgb ? Flags::Format::RGBA8_SRGB
                               : Flags::Format::RGBA8_UNORM;
        }
        
        m_textures[i] = m_rm->create<Texture>({
            .dimensions = {
                info.width,
                info.height,
                1
            },
            .format = format,
            .usage = Flags::ImageUsage::IU_SAMPLED |
                     Flags::ImageUsage::IU_TRANSFER_DST
        });
    }
}

void SceneManager::initDescriptorSets(VulkanDevice* device){
    // global (set = 0)
    m_globalDescriptorSetLayout = m_rm->create<DescriptorSetLayout>({
        .textures = {
            {.binding = 3, .count = (uint32)m_textures.size()},
        },
        .buffers = {
            {.binding = 0, .type = Flags::DescriptorType::STORAGE_BUFFER},
            {.binding = 1, .type = Flags::DescriptorType::STORAGE_BUFFER},
            {.binding = 2, .type = Flags::DescriptorType::STORAGE_BUFFER}
        }
    });
    m_globalDescriptorSet = m_rm->create<DescriptorSet>({
        .textures = {
            {.textures = m_textures}
        },
        .buffers = {
            {.buffer = m_positionsBuffer},
            {.buffer = m_attributesBuffer},
            {.buffer = m_materialsBuffer}
        },
        .layout = m_globalDescriptorSetLayout
    });
    // per-frame (set = 1)
    m_frameDescriptorSetLayout = m_rm->create<DescriptorSetLayout>({
        .buffers = {
            {.binding = 0, .type = Flags::DescriptorType::UNIFORM_BUFFER},
            {.binding = 1, .type = Flags::DescriptorType::UNIFORM_BUFFER},
            {.binding = 2, .type = Flags::DescriptorType::STORAGE_BUFFER},
            {.binding = 3, .type = Flags::DescriptorType::STORAGE_BUFFER},
            {.binding = 4, .type = Flags::DescriptorType::STORAGE_BUFFER}
        }
    });
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        
        Buffer* scene = m_rm->get<Buffer>(m_sceneBuffer);
        Buffer* cameras = m_rm->get<Buffer>(m_cameraBuffer);
        Buffer* lights = m_rm->get<Buffer>(m_lightsBuffer);
        Buffer* transforms = m_rm->get<Buffer>(m_transformBuffer);
        Buffer* draws = m_rm->get<Buffer>(m_drawDataBuffer);
        uint32 sceneFrameSize = ((scene->byteSize)/MAX_FRAME_COUNT);
        uint32 camerasFrameSize = ((cameras->byteSize)/MAX_FRAME_COUNT);
        uint32 lightsFrameSize = ((lights->byteSize)/MAX_FRAME_COUNT);
        uint32 transformsFrameSize = ((transforms->byteSize)/MAX_FRAME_COUNT);
        uint32 drawsFrameSize = ((draws->byteSize)/MAX_FRAME_COUNT);
        uint32 sceneOffset = i*sceneFrameSize;
        uint32 camerasOffset = i*camerasFrameSize;
        uint32 lightsOffset = i*lightsFrameSize;
        uint32 transformsOffset = i*transformsFrameSize;
        uint32 drawsOffset = i*drawsFrameSize;
        m_frameDescriptorSets[i] = m_rm->create<DescriptorSet>({
            .buffers = {
                {.buffer = m_sceneBuffer, .byteOffset = sceneOffset, .byteSize = sceneFrameSize},
                {.buffer = m_cameraBuffer, .byteOffset = camerasOffset, .byteSize = camerasFrameSize},
                {.buffer = m_lightsBuffer, .byteOffset = lightsOffset, .byteSize = lightsFrameSize},
                {.buffer = m_transformBuffer, .byteOffset = transformsOffset, .byteSize = transformsFrameSize},
                {.buffer = m_drawDataBuffer, .byteOffset = drawsOffset, .byteSize = drawsFrameSize}
            },
            .layout = m_frameDescriptorSetLayout
        });
    }
}


void SceneManager::reset(uint32 frame) {
    m_batchManagers[frame % MAX_FRAME_COUNT].reset();
    m_cameras[frame % MAX_FRAME_COUNT].reset();
    m_transforms[frame % MAX_FRAME_COUNT].reset();
    m_lights[frame % MAX_FRAME_COUNT].reset();
    m_drawData[frame % MAX_FRAME_COUNT].reset();
    m_sceneData[frame % MAX_FRAME_COUNT].reset();
    m_sceneData[frame % MAX_FRAME_COUNT].insert({});
    // m_assetLoader.clear(); TODO
}

void SceneManager::destroy(){
    // destroy per-frame batch managers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++)
        m_batchManagers[i].destroy();

    // destroy per-frame tempbuffers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_drawData[i].reset();
        m_cameras[i].reset();
        m_sceneData[i].reset();
        m_lights[i].reset();
        m_transforms[i].reset();
    }

    // destroy per-frame resources
    m_rm->remove<Buffer>(m_lightsBuffer);
    m_rm->remove<Buffer>(m_transformBuffer);
    m_rm->remove<Buffer>(m_drawDataBuffer);
    m_rm->remove<Buffer>(m_cameraBuffer);
    m_rm->remove<Buffer>(m_sceneBuffer);
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++)
        m_rm->remove<DescriptorSet>(m_frameDescriptorSets[i]);
    m_rm->remove<DescriptorSetLayout>(m_frameDescriptorSetLayout);
    
    // destroy global resources
    m_rm->remove<Buffer>(m_positionsBuffer);
    m_rm->remove<Buffer>(m_attributesBuffer);
    m_rm->remove<Buffer>(m_indexBuffer);
    m_rm->remove<Buffer>(m_materialsBuffer);
    for (Handle<Texture> texture : m_textures)
        m_rm->remove<Texture>(texture);
    m_rm->remove<DescriptorSet>(m_globalDescriptorSet);
    m_rm->remove<DescriptorSetLayout>(m_globalDescriptorSetLayout);
    
    m_destroyed = true;
    SprLog::info("[SceneManager] [destroy] destroyed...");
}

}