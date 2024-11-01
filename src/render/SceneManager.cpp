#include "SceneManager.h"
#include "SDL_timer.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "memory/TempBuffer.h"
#include "scene/GfxAssetLoader.h"
#include "scene/Material.h"
#include "scene/Mesh.h"
#include "vulkan/TextureTranscoder.h"
#include "vulkan/gfx_vulkan_core.h"
#include "vulkan/resource/ResourceFlags.h"
#include "vulkan/UploadHandler.h"
#include "resource/SprResourceManager.h"
#include <cctype>
#include "scene/SceneData.h"
#include "debug/SprLog.h"
#include <iostream>
#include <string>
#include<unistd.h>  


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


void SceneManager::insertDraws(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags, uint32 transformIndex, bool sharedMaterial){
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

void SceneManager::removeDraws(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags, uint32 transformIndex){
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
            .materialFlags  = materialsFlags[i],
            .indexCount     = meshInfo.indexCount,
            .firstIndex     = meshInfo.firstIndex,
            .drawDataOffset = 0,
            .drawCount      = 1
        };

        m_batchManagers[frame % MAX_FRAME_COUNT].removeDraw(draw, batchInfo);
    }
}


void SceneManager::insertMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags, const Transform& transform){
    bool sharedMaterial = (meshIds.size() != materialsFlags.size() && materialsFlags.size() == 1);

    uint32 transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert(transform);
    m_idTransformIndexMap[id] = transformIndex;

    if (frame >= MAX_FRAME_COUNT)
        queueTransformUpdate(transformIndex);

    insertDraws(frame, id, meshIds, materialsFlags, transformIndex, sharedMaterial);
}

void SceneManager::insertMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, uint32 materialFlags , const Transform& transform){
    uint32 transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert(transform);
    m_idTransformIndexMap[id] = transformIndex;

    if (frame >= MAX_FRAME_COUNT)
        queueTransformUpdate(transformIndex);

    insertDraws(frame, id, meshIds, {materialFlags}, transformIndex, true);
}


void SceneManager::insertMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags){
    bool sharedMaterial = (meshIds.size() != materialsFlags.size() && materialsFlags.size() == 1);

    uint32 transformIndex = m_idTransformIndexMap[id];

    insertDraws(frame, id, meshIds, materialsFlags, transformIndex, sharedMaterial);
}

void SceneManager::insertMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, uint32 materialFlags){
    uint32 transformIndex = m_idTransformIndexMap[id];

    insertDraws(frame, id, meshIds, {materialFlags}, transformIndex, true);
}

void SceneManager::removeMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags){
    uint32 transformIndex = m_idTransformIndexMap[id];

    removeDraws(frame, id, meshIds, materialsFlags, transformIndex);
}


void SceneManager::updateMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, Span<uint32> materialsFlags, const Transform& transform){
    uint32 transformIndex = m_idTransformIndexMap[id];
    m_transforms[frame % MAX_FRAME_COUNT][transformIndex] = transform;
    for (uint32 i = 1; i < MAX_FRAME_COUNT; i++){
        m_transforms[(frame + i) % MAX_FRAME_COUNT][transformIndex] = transform;
    }

    if (frame >= MAX_FRAME_COUNT)
        queueTransformUpdate(transformIndex);
}

void SceneManager::updateMeshes(uint32 frame, uint32 id, Span<uint32> meshIds, uint32 materialFlags , const Transform& transform){
    uint32 transformIndex = m_idTransformIndexMap[id];
    m_transforms[frame % MAX_FRAME_COUNT][transformIndex] = transform;
    for (uint32 i = 1; i < MAX_FRAME_COUNT; i++){
        m_transforms[(frame + i) % MAX_FRAME_COUNT][transformIndex] = transform;
    }

    if (frame >= MAX_FRAME_COUNT)
        queueTransformUpdate(transformIndex);
}


void SceneManager::insertLights(uint32 frame, Span<const Light> lights){
    uint32 offset = m_lights[frame % MAX_FRAME_COUNT].insert(lights.data(), lights.size());
    Scene& scene = m_sceneData[frame % MAX_FRAME_COUNT][0];
    scene.lightCount += lights.size();
    
    // get most recently added directional light
    for (uint32 i = 0; i < lights.size(); i++){
        if (lights[i].type == DIRECTIONAL){
            scene.sunOffset = offset + i;
        }
    }
}


void SceneManager::updateCamera(uint32 frame, glm::vec2 screenDim, const Camera& camera){
    m_cameras[frame % MAX_FRAME_COUNT].insert(camera);

    // pre-compute viewProjection matrix
    glm::mat4 view = glm::lookAt(camera.pos, camera.pos + camera.dir, camera.up);
    glm::mat4 proj = glm::perspectiveFovZO(camera.fov, screenDim.x, screenDim.y, camera.far, camera.near);
    Scene& scene = m_sceneData[frame % MAX_FRAME_COUNT][0];
    scene.view = view;
    scene.proj = proj;
    scene.viewProj = {proj * view};
    scene.screenDimX = screenDim.x;
    scene.screenDimY = screenDim.y;
    scene.time = SDL_GetTicks();
}


void SceneManager::uploadGlobalResources(UploadHandler& uploadHandler){
    std::vector<TextureInfo>& textures = m_assetLoader.getTextureData();
    for (uint32 i = 0; i < m_assetLoader.getPrimitiveCounts().textureCount; i++){
        uploadHandler.uploadManagedTexture<uint8>(textures[i].data.handle(), m_textures[i]);
    }
    
    std::vector<TextureInfo>& cubemaps = m_assetLoader.getCubemapData();
    for (uint32 i = 0; i < m_assetLoader.getPrimitiveCounts().cubemapCount; i++){
        uploadHandler.uploadManagedTexture<uint8>(cubemaps[i].data.handle(), m_cubemaps[i]);
    }
    
    uploadHandler.uploadManagedBuffer<VertexAttributes>(m_assetLoader.getVertexAttributeData(), m_attributesBuffer);
    uploadHandler.uploadManagedBuffer<VertexPosition>(m_assetLoader.getVertexPositionData(), m_positionsBuffer);
    uploadHandler.uploadManagedBuffer<uint32>(m_assetLoader.getVertexIndicesData(), m_indexBuffer);
    uploadHandler.uploadManagedBuffer<MaterialData>(m_assetLoader.getMaterialData(), m_materialsBuffer);
}

void SceneManager::uploadPerFrameResources(UploadHandler& uploadHandler, uint32 frame){
    uploadHandler.uploadDyanmicBuffer<Scene>({m_sceneData[frame % MAX_FRAME_COUNT]}, m_sceneBuffer);
    uploadHandler.uploadDyanmicBuffer<Camera>({m_cameras[frame % MAX_FRAME_COUNT]}, m_cameraBuffer);
    uploadHandler.uploadDyanmicBuffer<Light>({m_lights[frame % MAX_FRAME_COUNT]}, m_lightsBuffer);

    if (frame < MAX_FRAME_COUNT){
        uploadHandler.uploadDyanmicBuffer<Transform>({m_transforms[frame % MAX_FRAME_COUNT]}, m_transformBuffer);
    } else {
        for (TransformUpdate& update : m_transformUpdates){
            if (update.budget <= 0)
                continue;
            uploadHandler.uploadSparseBuffer<Transform>({m_transforms[frame % MAX_FRAME_COUNT]}, m_transformBuffer, update.index, update.index);
            update.budget--;
        }
    }
    
    m_batchManagers[frame % MAX_FRAME_COUNT].getDrawData(m_drawData[frame % MAX_FRAME_COUNT]);
    uploadHandler.uploadDyanmicBuffer<DrawData>({m_drawData[frame % MAX_FRAME_COUNT]}, m_drawDataBuffer);
}


void SceneManager::queueTransformUpdate(uint32 index){
    // check to see if this index is already being updated
    bool found = false;
    uint32 updatesIndex = 0;
    for (TransformUpdate& update : m_transformUpdates){
        if (update.index == index){
            found = true;
            update.budget = MAX_FRAME_COUNT;
            return;
        }

        if (update.budget <= 0) // passively remove completed updates
            m_updatesFreelist.push_back(updatesIndex);
        
        updatesIndex++;
    }

    // not found, add it
    if (!found){
        if (m_updatesFreelist.size()){
            m_transformUpdates[m_updatesFreelist.back()] = {index, MAX_FRAME_COUNT};
            m_updatesFreelist.pop_back();
        } else {
            m_transformUpdates.push_back({index, MAX_FRAME_COUNT});
        }
    }
}


Handle<DescriptorSet> SceneManager::getGlobalDescriptorSet(){
    return m_globalDescriptorSet;
}

Handle<DescriptorSetLayout> SceneManager::getGlobalDescriptorSetLayout(){
    return m_globalDescriptorSetLayout;
}

Handle<DescriptorSet> SceneManager::getPerFrameDescriptorSet(){
    return m_frameDescriptorSet;
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

Scene& SceneManager::getScene(uint32 frame){
    return m_sceneData[frame % MAX_FRAME_COUNT][0];
}

Camera& SceneManager::getCamera(uint32 frame){
    if (m_cameras[frame % MAX_FRAME_COUNT].getSize() == 0){
        m_cameras[frame % MAX_FRAME_COUNT].insert(Camera{});
    }
    return m_cameras[frame % MAX_FRAME_COUNT][0];
}

Light& SceneManager::getSunLight(uint32 frame){
    uint32 sunOffset = m_sceneData[frame % MAX_FRAME_COUNT][0].sunOffset;
    return m_lights[frame % MAX_FRAME_COUNT][sunOffset];
}


void SceneManager::initializeAssets(SprResourceManager &rm, VulkanDevice* device){
    m_meshInfo = m_assetLoader.loadAssets(rm, m_rm, device);
    m_assetLoader.unloadBuffers(rm);
    rm.destroyBuffers();

    PrimitiveCounts counts = m_assetLoader.getPrimitiveCounts();

    initTextures(counts, device);
    initBuffers(counts, device);
    initDescriptorSets(device);
    
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_batchManagers[i].setQuadInfo({
            .indexCount = m_meshInfo[1].indexCount,
            .firstIndex = m_meshInfo[1].firstIndex,
            .drawCount = 1
        }, m_meshInfo[1].vertexOffset);

        m_batchManagers[i].setCubeInfo({
            .indexCount = m_meshInfo[2].indexCount,
            .firstIndex = m_meshInfo[2].firstIndex,
            .drawCount = 1
        }, m_meshInfo[2].vertexOffset);
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
        .byteSize = (uint32) (counts.indexCount * sizeof(uint32)),
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

void SceneManager::initTextures(PrimitiveCounts counts, VulkanDevice* device){
    m_textures.resize(counts.textureCount);
    std::vector<TextureInfo>& textureData = m_assetLoader.getTextureData();
    for (uint32 i = 0; i < counts.textureCount; i++){
        TextureInfo& info = textureData[i];

        m_textures[i] = m_rm->create<Texture>({
            .dimensions = {
                info.width,
                info.height,
                1
            },
            .format = info.format,
            .usage = Flags::ImageUsage::IU_SAMPLED |
                     Flags::ImageUsage::IU_TRANSFER_DST,
            .view = { 
                .mips = info.mipCount,
                .layers = info.layerCount
            }
        });
    }

    m_cubemaps.resize(counts.cubemapCount);
    std::vector<TextureInfo>& cubemapData = m_assetLoader.getCubemapData();
    for (uint32 i = 0; i < counts.cubemapCount; i++){
        TextureInfo& info = cubemapData[i];

        m_cubemaps[i] = m_rm->create<Texture>({
            .dimensions = {
                info.width,
                info.height,
                1
            },
            .format = info.format,
            .usage = Flags::ImageUsage::IU_SAMPLED |
                     Flags::ImageUsage::IU_TRANSFER_DST,
            .view = { 
                .mips = info.mipCount,
                .layers = info.layerCount
            }
        });
    }
}

void SceneManager::initDescriptorSets(VulkanDevice* device){
    // global (set = 0)
    m_globalDescriptorSetLayout = m_rm->create<DescriptorSetLayout>({
        .textures = {
            {.binding = 3, .count = (uint32)m_textures.size()},
            {.binding = 4, .count = (uint32)m_cubemaps.size()},
        },
        .buffers = {
            {.binding = 0, .type = Flags::DescriptorType::STORAGE_BUFFER},
            {.binding = 1, .type = Flags::DescriptorType::STORAGE_BUFFER},
            {.binding = 2, .type = Flags::DescriptorType::STORAGE_BUFFER}
        }
    });
    m_globalDescriptorSet = m_rm->create<DescriptorSet>({
        .textures = {
            {.textures = m_textures},
            {.textures = m_cubemaps}
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
    Buffer* scene = m_rm->get<Buffer>(m_sceneBuffer);
    Buffer* cameras = m_rm->get<Buffer>(m_cameraBuffer);
    Buffer* lights = m_rm->get<Buffer>(m_lightsBuffer);
    Buffer* transforms = m_rm->get<Buffer>(m_transformBuffer);
    Buffer* draws = m_rm->get<Buffer>(m_drawDataBuffer);
    m_frameDescriptorSet = m_rm->create<DescriptorSet>({
        .buffers = {
            {.dynamicBuffer = m_sceneBuffer, .byteSize = scene->byteSize},
            {.dynamicBuffer = m_cameraBuffer, .byteSize = cameras->byteSize},
            {.dynamicBuffer = m_lightsBuffer, .byteSize = lights->byteSize},
            {.dynamicBuffer = m_transformBuffer, .byteSize = transforms->byteSize},
            {.dynamicBuffer = m_drawDataBuffer, .byteSize = draws->byteSize}
        },
        .layout = m_frameDescriptorSetLayout
    });
    
}


void SceneManager::reset(uint32 frame) {
    m_batchManagers[frame % MAX_FRAME_COUNT].reset();
    m_cameras[frame % MAX_FRAME_COUNT].clear();
    //m_transforms[frame % MAX_FRAME_COUNT].clear();
    m_lights[frame % MAX_FRAME_COUNT].clear();
    m_drawData[frame % MAX_FRAME_COUNT].clear();
    m_sceneData[frame % MAX_FRAME_COUNT].clear();
    m_sceneData[frame % MAX_FRAME_COUNT].insert({});
    m_assetLoader.clear();
}

void SceneManager::destroy(){
    // destroy per-frame batch managers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++)
        m_batchManagers[i].destroy();

    // destroy per-frame tempbuffers
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_drawData[i].destroy();
        m_cameras[i].destroy();
        m_sceneData[i].destroy();
        m_lights[i].destroy();
        m_transforms[i].destroy();
    }

    // destroy per-frame resources
    m_rm->remove<Buffer>(m_lightsBuffer);
    m_rm->remove<Buffer>(m_transformBuffer);
    m_rm->remove<Buffer>(m_drawDataBuffer);
    m_rm->remove<Buffer>(m_cameraBuffer);
    m_rm->remove<Buffer>(m_sceneBuffer);
    m_rm->remove<DescriptorSet>(m_frameDescriptorSet);
    m_rm->remove<DescriptorSetLayout>(m_frameDescriptorSetLayout);
    
    // destroy global resources
    m_rm->remove<Buffer>(m_positionsBuffer);
    m_rm->remove<Buffer>(m_attributesBuffer);
    m_rm->remove<Buffer>(m_indexBuffer);
    m_rm->remove<Buffer>(m_materialsBuffer);
    for (Handle<Texture> texture : m_textures)
        m_rm->remove<Texture>(texture);
    for (Handle<Texture> cubemap : m_cubemaps)
        m_rm->remove<Texture>(cubemap);
    m_rm->remove<DescriptorSet>(m_globalDescriptorSet);
    m_rm->remove<DescriptorSetLayout>(m_globalDescriptorSetLayout);
    
    m_destroyed = true;
    SprLog::info("[SceneManager] [destroy] destroyed...");
}

}