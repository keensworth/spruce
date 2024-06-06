#include "RenderCoordinator.h"
#include "glm/geometric.hpp"
#include "renderers/DebugMeshRenderer.h"
#include "renderers/GTAORenderer.h"
#include "renderers/SkyboxRenderer.h"
#include "renderers/SunShadowRenderer.h"
#include "renderers/VolumetricLightRenderer.h"
#include "vulkan/ImGuiRenderer.h"
#include "vulkan/gfx_vulkan_core.h"
#include "vulkan/resource/ResourceTypes.h"
#include "SceneManager.h"
#include "../interface/SprWindow.h"

namespace spr::gfx {

RenderCoordinator::RenderCoordinator(){}
RenderCoordinator::RenderCoordinator(SprWindow* window){
    m_window = window;
    m_frameId = 0;
}

RenderCoordinator::~RenderCoordinator(){
    SprLog::info("[RenderCoordinator] [destroy] destroyed...");
}

void RenderCoordinator::init(VulkanRenderer* renderer, VulkanResourceManager* rm){
    m_rm = rm;
    m_renderer = renderer;
}


void RenderCoordinator::render(SceneManager& sceneManager){
    // begin frame
    RenderFrame& frame = m_renderer->beginFrame(m_rm);

    // early return if invalid swapchain detected
    if (m_renderer->invalidSwapchain())
        return;

    BatchManager& batchManager = sceneManager.getBatchManager(m_frameId);
    uploadSceneData(sceneManager);

    // get common batches
    std::vector<Batch> allMaterialBatches;
    batchManager.getBatches({.hasAny = MTL_ALL}, allMaterialBatches);

    // render offscreen renderpasses
    CommandBuffer& offscreenCB = m_renderer->beginGraphicsCommands(CommandType::OFFSCREEN);
    offscreenCB.bindIndexBuffer(sceneManager.getIndexBuffer());
    {   
        // depth pre-pass
        m_depthPrepassRenderer.render(offscreenCB, allMaterialBatches);

        // cascaded shadows
        m_sunShadowRenderer.render(offscreenCB, allMaterialBatches);

        // volumetric lighting
        m_volumetricLightRenderer.render(offscreenCB, batchManager);

        // ambient occlusion
        m_gtaoRenderer.render(offscreenCB, batchManager);

        // blur ao output
        m_blurRenderer.render(offscreenCB, batchManager);

        // render currently enabled output renderer
        uint32 visible = m_imguiRenderer.state.visible;
        
        if (visible & (RenderState::LIT_MESH | RenderState::FXAA)) {
            m_litMeshRenderer.render(offscreenCB, allMaterialBatches);
            m_skyboxRenderer.render(offscreenCB, batchManager);
            m_fxaaRenderer.render(offscreenCB, batchManager);
        } 
        else if (visible & RenderState::DEBUG_MESH) {
            m_debugMeshRenderer.render(offscreenCB, allMaterialBatches);
        }
        else if (visible & RenderState::DEBUG_NORMALS) {
            m_debugNormalsRenderer.render(offscreenCB, allMaterialBatches);
        }
        else if (visible & RenderState::DEBUG_SHADOW_CASCADES) {
            m_debugCascadesRenderer.render(offscreenCB, allMaterialBatches);
        }
        else if (visible & RenderState::UNLIT_MESH) {
            m_unlitMeshRenderer.render(offscreenCB, allMaterialBatches);
        }
        else { // TEST
            m_testRenderer.render(offscreenCB, batchManager);
        }
        
        // render imgui
        m_imguiRenderer.render(offscreenCB, batchManager);
    }
    offscreenCB.submit();

    // render to swapchain image
    CommandBuffer& mainCB = m_renderer->beginGraphicsCommands(CommandType::MAIN);
    mainCB.bindIndexBuffer(sceneManager.getIndexBuffer());
    {
        m_frameRenderer.render(mainCB, batchManager);
    }
    mainCB.submit();

    // present result
    m_renderer->present(frame);
    m_frameId = m_renderer->getFrameId();

    updateUI(offscreenCB);
}


void RenderCoordinator::uploadSceneData(SceneManager& sceneManager){
    UploadHandler& uploadHandler = m_renderer->beginTransferCommands();

    // global data
    if (!m_sceneInitialized){
        m_sceneInitialized = true;
        sceneManager.uploadGlobalResources(uploadHandler);
    }

    // get and update scene data
    Scene& scene = sceneManager.getScene(m_frameId);
    Camera& camera = sceneManager.getCamera(m_frameId);
    Light& sunLight = sceneManager.getSunLight(m_frameId);
    // override w/ imgui input
    sunLight.color = m_imguiRenderer.state.lightColor;
    sunLight.dir = glm::normalize(m_imguiRenderer.state.lightDir);
    scene.exposure = m_imguiRenderer.state.exposure;

    // per-frame data
    sceneManager.uploadPerFrameResources(uploadHandler, m_frameId);

    // renderer specific
    m_sunShadowRenderer.uploadData(scene, camera, sunLight, uploadHandler, m_imguiRenderer.state.cascadeLambda);

    uploadHandler.submit();
}

void RenderCoordinator::updateUI(CommandBuffer& offscreenCB){
    // need to change input to copy shader
    if (m_imguiRenderer.state.dirtyOutput){
        Handle<TextureAttachment> output;
        uint32 visible = m_imguiRenderer.state.visible;

        if (visible & RenderState::TEST){
            output = m_testRenderer.getAttachment();
        } else if (visible & RenderState::DEBUG_MESH){
            output = m_debugMeshRenderer.getAttachment();
        } else if (visible & RenderState::DEBUG_NORMALS){
            output = m_debugNormalsRenderer.getAttachment();
        } else if (visible & RenderState::DEPTH_PREPASS){
            output = m_depthPrepassRenderer.getDepthAttachment();
        } else if (visible & RenderState::SHADOW_CASCADES){
            output = m_sunShadowRenderer.getDepthAttachments()[m_imguiRenderer.state.shadowSelection];
        } else if (visible & RenderState::DEBUG_SHADOW_CASCADES){
            output = m_debugCascadesRenderer.getAttachment();
        } else if (visible & RenderState::GTAO_PASS){
            output = m_gtaoRenderer.getAttachment();
        } else if (visible & RenderState::VOLUMETRIC_LIGHT){
            output = m_volumetricLightRenderer.getAttachment();
        } else if (visible & RenderState::FXAA){
            output = m_fxaaRenderer.getAttachment();
        } else if (visible & RenderState::UNLIT_MESH){
            output = m_unlitMeshRenderer.getAttachment();
        } else if (visible & RenderState::BLUR_PASS){
            output = m_blurRenderer.getAttachment();
        } else{ // LIT_MESH
            output = m_litMeshRenderer.getAttachment();
        }
        m_imguiRenderer.setInput(output);
        m_imguiRenderer.state.dirtyOutput = false;
    }
    if (m_imguiRenderer.state.dirtyShader){
        Handle<Shader> reload;
        uint32 visible = m_imguiRenderer.state.visible;
        RenderState::Shader shaderToReload = m_imguiRenderer.state.shaderToReload;

        if (shaderToReload == RenderState::TEST){
            reload = m_testRenderer.getShader();
        } else if (shaderToReload == RenderState::DEBUG_MESH){
            reload = m_debugMeshRenderer.getShader();
        } else if (shaderToReload == RenderState::DEBUG_NORMALS){
            reload = m_debugNormalsRenderer.getShader();
        } else if (shaderToReload == RenderState::DEPTH_PREPASS){
            reload = m_depthPrepassRenderer.getShader();
        } else if (shaderToReload == RenderState::SHADOW_CASCADES){
            reload = m_sunShadowRenderer.getShader();
        } else if (shaderToReload == RenderState::DEBUG_SHADOW_CASCADES){
            reload = m_debugCascadesRenderer.getShader();
        } else if (shaderToReload == RenderState::GTAO_PASS){
            reload = m_gtaoRenderer.getShader();
        } else if (shaderToReload == RenderState::VOLUMETRIC_LIGHT){
            reload = m_volumetricLightRenderer.getShader();
        } else if (shaderToReload == RenderState::FXAA){
            reload = m_fxaaRenderer.getShader();
        }else if (shaderToReload == RenderState::BLUR_PASS){
            reload = m_blurRenderer.getShader();
        } else if (shaderToReload == RenderState::UNLIT_MESH){
            reload = m_unlitMeshRenderer.getShader();
        } else if (shaderToReload == RenderState::LIT_MESH){
            reload = m_litMeshRenderer.getShader();
        } else {
            return;
        }

        offscreenCB.waitFence();
        m_rm->recreate<Shader>(reload, true);
        
        if (visible & RenderState::LIT_MESH)
            m_rm->recreate<Shader>(m_skyboxRenderer.getShader(), true);

        m_imguiRenderer.state.dirtyShader = false;
    }
}


void RenderCoordinator::initRenderers(SceneManager& sceneManager){
    glm::uvec3 windowDim = {m_window->width(), m_window->height(), 1};

    Handle<DescriptorSet> globalDescSet = sceneManager.getGlobalDescriptorSet();
    Handle<DescriptorSetLayout> globalDescSetLayout = sceneManager.getGlobalDescriptorSetLayout();
    Handle<DescriptorSet> frameDescSets = sceneManager.getPerFrameDescriptorSet();
    Handle<DescriptorSetLayout> frameDescSetLayout = sceneManager.getPerFrameDescriptorSetLayout();

    // offscreen renderers
    m_testRenderer = TestRenderer(*m_rm, *m_renderer, windowDim);
    m_testRenderer.init(
        globalDescSet,
        globalDescSetLayout);

    m_depthPrepassRenderer = DepthPrepassRenderer(*m_rm, *m_renderer, windowDim);
    m_depthPrepassRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout);

    m_sunShadowRenderer = SunShadowRenderer(*m_rm, *m_renderer);
    m_sunShadowRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout);

    m_volumetricLightRenderer = VolumetricLightRenderer(*m_rm, *m_renderer, windowDim);
    m_volumetricLightRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_depthPrepassRenderer.getDepthAttachment(),
        m_sunShadowRenderer.getDepthAttachments(),
        m_sunShadowRenderer.getShadowData());

    m_gtaoRenderer = GTAORenderer(*m_rm, *m_renderer, windowDim);
    m_gtaoRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_depthPrepassRenderer.getDepthAttachment());

    m_blurRenderer = BlurRenderer(*m_rm, *m_renderer, windowDim);
    m_blurRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_gtaoRenderer.getAttachment());

    m_debugMeshRenderer = DebugMeshRenderer(*m_rm, *m_renderer, windowDim);
    m_debugMeshRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_depthPrepassRenderer.getDepthAttachment());

    m_debugNormalsRenderer = DebugNormalsRenderer(*m_rm, *m_renderer, windowDim);
    m_debugNormalsRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_depthPrepassRenderer.getDepthAttachment());

    m_debugCascadesRenderer = DebugCascadesRenderer(*m_rm, *m_renderer, windowDim);
    m_debugCascadesRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_depthPrepassRenderer.getDepthAttachment(),
        m_sunShadowRenderer.getShadowData());

    m_unlitMeshRenderer = UnlitMeshRenderer(*m_rm, *m_renderer, windowDim);
    m_unlitMeshRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_depthPrepassRenderer.getDepthAttachment());

    m_litMeshRenderer = LitMeshRenderer(*m_rm, *m_renderer, windowDim);
    m_litMeshRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_depthPrepassRenderer.getDepthAttachment(),
        m_blurRenderer.getAttachment(),
        m_sunShadowRenderer.getDepthAttachments(),
        m_sunShadowRenderer.getShadowData(),
        m_volumetricLightRenderer.getAttachment());

    m_skyboxRenderer = SkyboxRenderer(*m_rm, *m_renderer, windowDim);
    m_skyboxRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_litMeshRenderer.m_descriptorSet,
        m_litMeshRenderer.m_descriptorSetLayout,
        m_depthPrepassRenderer.getDepthAttachment(),
        m_litMeshRenderer.m_attachment);

    m_fxaaRenderer = FXAARenderer(*m_rm, *m_renderer, windowDim);
    m_fxaaRenderer.init(
        globalDescSet,
        globalDescSetLayout,
        frameDescSets,
        frameDescSetLayout,
        m_litMeshRenderer.m_attachment);

    m_imguiRenderer = ImGuiRenderer(*m_rm, *m_renderer, m_window, windowDim);
    m_imguiRenderer.init(
        globalDescSet,
        globalDescSetLayout);
    m_imguiRenderer.setInput(m_fxaaRenderer.m_attachment);
    
    // swapchain renderer
    m_frameRenderer = FrameRenderer(*m_rm, *m_renderer, m_window, windowDim);
    m_frameRenderer.init(
        m_renderer->getDisplay().getImageViews(), 
        globalDescSet,
        globalDescSetLayout);
    m_frameRenderer.setInput(m_imguiRenderer.getAttachment());
}


void RenderCoordinator::onResize(){
    // wait on all GPU work to cease
    m_renderer->wait();

    // destroy framebuffer built over swapchin
    m_frameRenderer.destroyFramebuffer();

    // rebuild swapchain and image views
    m_renderer->onResize();

    glm::uvec2 windowDim = {m_window->width(), m_window->height()};
    m_rm->updateScreenDim(windowDim);

    // rebuild all screen-sized framebuffers
    m_frameRenderer.updateImageViews(m_renderer->getDisplay().getImageViews());
    m_rm->recreate<RenderPass>(m_frameRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_depthPrepassRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_imguiRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_skyboxRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_testRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_debugMeshRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_debugNormalsRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_debugCascadesRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_sunShadowRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_volumetricLightRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_blurRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_gtaoRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_fxaaRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_unlitMeshRenderer.getRenderPass(), windowDim);
    m_rm->recreate<RenderPass>(m_litMeshRenderer.getRenderPass(), windowDim);

    // make updates to descriptors
    m_frameRenderer.setInput(m_imguiRenderer.getAttachment());
    m_imguiRenderer.setInput(m_fxaaRenderer.getAttachment());
    m_volumetricLightRenderer.updateDescriptorSet(
        m_depthPrepassRenderer.getDepthAttachment(),
        m_sunShadowRenderer.getDepthAttachments(),
        m_sunShadowRenderer.getShadowData());
    m_blurRenderer.updateDescriptorSet(m_gtaoRenderer.getAttachment());
    m_gtaoRenderer.updateDescriptorSet(m_depthPrepassRenderer.getDepthAttachment());
    m_litMeshRenderer.updateDescriptorSet(
        m_depthPrepassRenderer.getDepthAttachment(),
        m_blurRenderer.getAttachment(),
        m_sunShadowRenderer.getDepthAttachments(),
        m_sunShadowRenderer.getShadowData(),
        m_volumetricLightRenderer.getAttachment());
    m_fxaaRenderer.updateDescriptorSet(m_litMeshRenderer.getAttachment());
    m_skyboxRenderer.updateDescriptorSet(m_litMeshRenderer.m_descriptorSet);
}


void RenderCoordinator::destroy(){
    // teardown renderers
    m_frameRenderer.destroy();
    m_imguiRenderer.destroy();
    m_skyboxRenderer.destroy();
    m_testRenderer.destroy();
    m_debugMeshRenderer.destroy();
    m_debugNormalsRenderer.destroy();
    m_debugCascadesRenderer.destroy();
    m_depthPrepassRenderer.destroy();
    m_sunShadowRenderer.destroy();
    m_volumetricLightRenderer.destroy();
    m_blurRenderer.destroy();
    m_fxaaRenderer.destroy();
    m_gtaoRenderer.destroy();
    m_unlitMeshRenderer.destroy();
    m_litMeshRenderer.destroy();
}

}