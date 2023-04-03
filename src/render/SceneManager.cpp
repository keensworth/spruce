#include "SceneManager.h"
#include "scene/GfxAssetLoader.h"
#include "scene/Material.h"
#include "scene/Mesh.h"
#include "vulkan/gfx_vulkan_core.h"
#include "vulkan/resource/ResourceTypes.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include <cctype>

namespace spr::gfx {

SceneManager::SceneManager(){}

SceneManager::SceneManager(VulkanResourceManager& rm){  
    m_rm = &rm;
    m_assetLoader = GfxAssetLoader();
    
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


void SceneManager::insertMesh(uint32 frame, uint32 meshId, uint32 materialFlags, Transform& transform){
    // get mesh data and fill draw
    MeshInfo& meshInfo = m_meshInfo[meshId];
    DrawData draw = {
        .vertexOffset   = meshInfo.vertexOffset,  
        .materialIndex  = meshInfo.materialIndex, 
        .transformIndex = m_transforms[frame % MAX_FRAME_COUNT].insert(transform)
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


void SceneManager::insertLight(uint32 frame, Light& light){
    m_lights[frame % MAX_FRAME_COUNT].insert(light);
}


void SceneManager::updateCamera(uint32 frame, Camera& camera){
    m_cameras[frame % MAX_FRAME_COUNT].insert(camera);
}


void SceneManager::uploadGlobalResources(UploadHandler& uploadHandler){
    uploadHandler.uploadBuffer(m_assetLoader.getVertexPositionData(), m_positionsBuffer);
    uploadHandler.uploadBuffer(m_assetLoader.getVertexPositionData(), m_attributesBuffer);
    uploadHandler.uploadBuffer(m_assetLoader.getVertexPositionData(), m_materialsBuffer);
    uploadHandler.uploadBuffer(m_assetLoader.getVertexPositionData(), m_indexBuffer);

    std::vector<TextureInfo> textures = m_assetLoader.getTextureData();
    for (uint32 i = 0; i < m_assetLoader.getPrimitiveCounts().textureCount; i++){
        TextureInfo texture = textures[i];
        uploadHandler.uploadTexture(texture.data, m_textures[i]);
    }

    m_assetLoader.clear();
}

void SceneManager::uploadPerFrameResources(UploadHandler& uploadHandler, uint32 frame){
    uploadHandler.uploadDyanmicBuffer(m_sceneData[frame % MAX_FRAME_COUNT], m_sceneBuffer);
    uploadHandler.uploadDyanmicBuffer(m_cameras[frame % MAX_FRAME_COUNT], m_cameraBuffer);
    uploadHandler.uploadDyanmicBuffer(m_lights[frame % MAX_FRAME_COUNT], m_lightsBuffer);
    uploadHandler.uploadDyanmicBuffer(m_transforms[frame % MAX_FRAME_COUNT], m_transformBuffer);
    uploadHandler.uploadDyanmicBuffer(m_drawData[frame % MAX_FRAME_COUNT], m_drawDataBuffer);
}


Handle<DescriptorSet> SceneManager::getGlobalDescriptorSet(){
    return m_globalDescriptorSet;
}

Handle<DescriptorSet> SceneManager::getPerFrameDescriptorSet(uint32 frame){
    return m_frameDescriptorSets[frame % MAX_FRAME_COUNT];
}

BatchManager& SceneManager::getBatchManager(uint32 frame) {
    return m_batchManagers[frame % MAX_FRAME_COUNT];
}


void SceneManager::initializeAssets(SprResourceManager &rm){
    m_meshInfo = m_assetLoader.loadAssets(rm);
    PrimitiveCounts counts = m_assetLoader.getPrimitiveCounts();
    initBuffers(counts);
    initTextures(counts.textureCount);
    initDescriptorSets();
    
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_batchManagers[i].setQuadBatch({
            .indexCount = m_meshInfo[1].indexCount,
            .firstIndex = m_meshInfo[1].firstIndex,
            .drawCount = 1
        });
    }
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

        m_textures[i] = m_rm->create<Texture>(TextureDesc{
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

void SceneManager::initDescriptorSets(){
    // global (set = 0)
    m_globalDescriptorSetLayout = m_rm->create<DescriptorSetLayout>(DescriptorSetLayoutDesc{
        .textures = {
            {.binding = 3},
        },
        .buffers = {
            {.binding = 0},
            {.binding = 1},
            {.binding = 2}
        }
    });
    m_globalDescriptorSet = m_rm->create<DescriptorSet>(DescriptorSetDesc{
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
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_frameDescriptorSetLayouts[i] = m_rm->create<DescriptorSetLayout>(DescriptorSetLayoutDesc{
            .buffers = {
                {.binding = 0},
                {.binding = 1},
                {.binding = 2},
                {.binding = 3},
                {.binding = 4}
            }
        });
        Buffer* scene = m_rm->get<Buffer>(m_sceneBuffer);
        Buffer* cameras = m_rm->get<Buffer>(m_sceneBuffer);
        Buffer* lights = m_rm->get<Buffer>(m_sceneBuffer);
        Buffer* transforms = m_rm->get<Buffer>(m_sceneBuffer);
        Buffer* draws = m_rm->get<Buffer>(m_sceneBuffer);
        m_frameDescriptorSets[i] = m_rm->create<DescriptorSet>(DescriptorSetDesc{
            .buffers = {
                {.buffer = m_sceneBuffer, .byteOffset = i*(scene->byteSize/MAX_FRAME_COUNT)},
                {.buffer = m_cameraBuffer, .byteOffset = i*(cameras->byteSize/MAX_FRAME_COUNT)},
                {.buffer = m_lightsBuffer, .byteOffset = i*(lights->byteSize/MAX_FRAME_COUNT)},
                {.buffer = m_transformBuffer, .byteOffset = i*(transforms->byteSize/MAX_FRAME_COUNT)},
                {.buffer = m_drawDataBuffer, .byteOffset = i*(draws->byteSize/MAX_FRAME_COUNT)}
            }
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
    m_rm->remove<Buffer>(m_lightsBuffer);
    m_rm->remove<Buffer>(m_transformBuffer);
    m_rm->remove<Buffer>(m_drawDataBuffer);
    m_rm->remove<Buffer>(m_cameraBuffer);
    m_rm->remove<Buffer>(m_sceneBuffer);
    for (uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_rm->remove<DescriptorSetLayout>(m_frameDescriptorSetLayouts[i]);
        m_rm->remove<DescriptorSet>(m_frameDescriptorSets[i]);
    }

    // destroy global resources
    m_rm->remove<Buffer>(m_positionsBuffer);
    m_rm->remove<Buffer>(m_attributesBuffer);
    m_rm->remove<Buffer>(m_indexBuffer);
    m_rm->remove<Buffer>(m_materialsBuffer);
    for (Handle<Texture> texture : m_textures)
        m_rm->remove<Texture>(texture);
    m_rm->remove<DescriptorSetLayout>(m_globalDescriptorSetLayout);
    m_rm->remove<DescriptorSet>(m_globalDescriptorSet);


    m_destroyed = true;
}

}