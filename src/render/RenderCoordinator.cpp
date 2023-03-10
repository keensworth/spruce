#include "RenderCoordinator.h"
#include "scene/Draw.h"
#include "scene/Material.h"
#include "scene/SceneData.h"
#include "scene/SceneManager.h"
#include "vulkan/CommandBuffer.h"
#include "vulkan/VulkanRenderer.h"
#include "vulkan/resource/ResourceTypes.h"
#include "vulkan/resource/VulkanResourceManager.h"
#include "vulkan/vulkan_core.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

RenderCoordinator::RenderCoordinator(Window* window){
    m_window = window;
    m_rm = VulkanResourceManager();
    m_renderer = VulkanRenderer(window, &m_rm);
    m_rm.init(m_renderer.getDevice(), m_renderer.getAllocator());

    initBuffers();
    initRenderers();
}

RenderCoordinator::~RenderCoordinator(){

}


void RenderCoordinator::render(SceneManager& sceneManager){
    // get current frame
    RenderFrame& frame = m_renderer.beginFrame();

    // upload scene resources
    uploadResources(sceneManager);

    BatchManager& batchManager = sceneManager.getBatchManager(m_frame);

    // PASS 1
    // PASS 2
    // PASS 3
    // ...
    // PASS N

    // render to swapchain image
    m_frameRenderer.render();

    // present result
    m_renderer.present(frame);
    m_frame = m_renderer.getFrameId();
}


void RenderCoordinator::uploadResources(SceneManager& sceneManager){
    UploadHandler& uploadHandler = m_renderer.beginTransferCommands();
    
    // get resources
    TempBuffer<Light>& lights = sceneManager.getLights(m_frame);
    TempBuffer<Transform>& transforms = sceneManager.getTransforms(m_frame);
    TempBuffer<DrawData>& drawData = sceneManager.getDrawData(m_frame);
    TempBuffer<Camera>& camera = sceneManager.getCamera(m_frame);
    TempBuffer<Scene>& scene = sceneManager.getScene(m_frame);

    // upload resources
    uploadHandler.uploadDyanmicBuffer(lights, m_lightsBuffer);
    uploadHandler.uploadDyanmicBuffer(transforms, m_transformBuffer);
    uploadHandler.uploadDyanmicBuffer(drawData, m_drawDataBuffer);
    uploadHandler.uploadDyanmicBuffer(camera, m_cameraBuffer);
    uploadHandler.uploadDyanmicBuffer(scene, m_sceneBuffer);

    uploadHandler.submit();
}


void RenderCoordinator::initRenderers(){
    // setup renderers
    // TODO
}


void RenderCoordinator::initBuffers(){
    Handle<Buffer> m_lightsBuffer = m_rm.create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_LIGHTS * MAX_FRAME_COUNT * sizeof(Light)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_transformBuffer = m_rm.create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_DRAWS * MAX_FRAME_COUNT * sizeof(Transform)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_drawDataBuffer = m_rm.create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_DRAWS * MAX_FRAME_COUNT * sizeof(DrawData)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_cameraBuffer = m_rm.create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_FRAME_COUNT * sizeof(Camera)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });

    Handle<Buffer> m_sceneBuffer = m_rm.create<Buffer>(BufferDesc{
        .byteSize = (uint32) (MAX_FRAME_COUNT * sizeof(Scene)),
        .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
                 Flags::BufferUsage::BU_TRANSFER_DST,
        .memType = DEVICE | HOST
    });
}


void RenderCoordinator::onResize(){
    m_renderer.onResize();

    // get all renderpasses
    //Handle<RenderPass> passA =  m_rendererA.getRenderpass();

    // rebuild all framebuffers
    //m_rm.recreate<RenderPass>(passA, glm::uvec2(m_window->width(), m_window->height()));

}


void RenderCoordinator::destroy(){

}

}