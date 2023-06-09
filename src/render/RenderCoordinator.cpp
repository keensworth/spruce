#include "RenderCoordinator.h"
#include "renderers/DebugMeshRenderer.h"
#include "renderers/GTAORenderer.h"
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
    BatchManager& batchManager = sceneManager.getBatchManager(m_frameId);
    uploadSceneData(sceneManager);

    // offscreen renderpasses
    CommandBuffer& offscreenCB = m_renderer->beginGraphicsCommands(CommandType::OFFSCREEN);
    offscreenCB.bindIndexBuffer(sceneManager.getIndexBuffer());
    {   
        // depth pre-pass
        m_depthPrepassRenderer.render(offscreenCB, batchManager);

        // ambient occlusion
        m_gtaoRenderer.render(offscreenCB, batchManager);

        // blur ao output
        m_blurRenderer.render(offscreenCB, batchManager);

        // render currently enabled renderer
        uint32 visible = m_imguiRenderer.state.visible;
        if (visible & RenderState::TEST)
            m_testRenderer.render(offscreenCB, batchManager);
        else if (visible & RenderState::DEBUG_MESH)
            m_debugMeshRenderer.render(offscreenCB, batchManager);
        else if (visible & RenderState::DEBUG_NORMALS)
            m_debugNormalsRenderer.render(offscreenCB, batchManager);
        else if (visible & RenderState::UNLIT_MESH)
            m_unlitMeshRenderer.render(offscreenCB, batchManager);
        else // LIT_MESH
            m_litMeshRenderer.render(offscreenCB, batchManager);
        
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

    // need to change input to copy shader
    if (m_imguiRenderer.state.dirtyOutput){
        Handle<TextureAttachment> output;
        uint32 visible = m_imguiRenderer.state.visible;

        if (visible & RenderState::TEST)
            output = m_testRenderer.getAttachment();

        else if (visible & RenderState::DEBUG_MESH)
            output = m_debugMeshRenderer.getAttachment();

        else if (visible & RenderState::DEBUG_NORMALS)
            output = m_debugNormalsRenderer.getAttachment();

        else if (visible & RenderState::DEPTH_PREPASS)
            output = m_depthPrepassRenderer.getDepthAttachment();

        else if (visible & RenderState::GTAO_PASS)
            output = m_gtaoRenderer.getAttachment();

        else if (visible & RenderState::UNLIT_MESH)
            output = m_unlitMeshRenderer.getAttachment();

        else if (visible & RenderState::BLUR_PASS)
            output = m_blurRenderer.getAttachment();

        else // LIT_MESH
            output = m_litMeshRenderer.getAttachment();
        
        m_imguiRenderer.setInput(output);
        m_imguiRenderer.state.dirtyOutput = false;
    }
    if (m_imguiRenderer.state.dirtyShader){
        Handle<Shader> reload;
        uint32 visible = m_imguiRenderer.state.visible;

        if (visible & RenderState::TEST){
            reload = m_testRenderer.getShader();
        } else if (visible & RenderState::DEBUG_MESH){
            reload = m_debugMeshRenderer.getShader();
        } else if (visible & RenderState::DEBUG_NORMALS){
            reload = m_debugNormalsRenderer.getShader();
        } else if (visible & RenderState::DEPTH_PREPASS){
            reload = m_depthPrepassRenderer.getShader();
        } else if (visible & RenderState::GTAO_PASS){
            reload = m_gtaoRenderer.getShader();
        } else if (visible & RenderState::BLUR_PASS){
            reload = m_blurRenderer.getShader();
        } else if (visible & RenderState::UNLIT_MESH){
            reload = m_unlitMeshRenderer.getShader();
        } else if (visible & RenderState::LIT_MESH){
            reload = m_litMeshRenderer.getShader();
        } else {
            return;
        }

        offscreenCB.waitFence();
        m_rm->recreate<Shader>(reload, true);
        m_imguiRenderer.state.dirtyShader = false;
    }
}


void RenderCoordinator::uploadSceneData(SceneManager& sceneManager){
    UploadHandler& uploadHandler = m_renderer->beginTransferCommands();

    // global
    if (!m_sceneInitialized){
        m_sceneInitialized = true;
        sceneManager.uploadGlobalResources(uploadHandler);
    }

    // per-frame
    sceneManager.uploadPerFrameResources(uploadHandler, m_frameId);

    uploadHandler.submit();
}

void RenderCoordinator::initRenderers(SceneManager& sceneManager){
    glm::uvec3 windowDim = {m_window->width(), m_window->height(), 1};

    Handle<DescriptorSet> globalDescSet = sceneManager.getGlobalDescriptorSet();
    Handle<DescriptorSetLayout> globalDescSetLayout = sceneManager.getGlobalDescriptorSetLayout();
    Handle<DescriptorSet>* frameDescSets = sceneManager.getPerFrameDescriptorSets();
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
        m_blurRenderer.getAttachment());

    m_imguiRenderer = ImGuiRenderer(*m_rm, *m_renderer, m_window, windowDim);
    m_imguiRenderer.init(
        globalDescSet,
        globalDescSetLayout);
    m_imguiRenderer.setInput(m_litMeshRenderer.getAttachment());

    // swapchain renderer
    m_frameRenderer = FrameRenderer(*m_rm, *m_renderer, m_window, windowDim);
    m_frameRenderer.init(
        m_renderer->getDisplay().getImageViews(), 
        globalDescSet,
        globalDescSetLayout);
    m_frameRenderer.setInput(m_imguiRenderer.getAttachment());
}


void RenderCoordinator::onResize(){
    m_renderer->onResize();

    glm::uvec2 windowDim = {m_window->width(), m_window->height()};

    // rebuild all framebuffers
    m_rm->recreate<RenderPass>(m_frameRenderer.getRenderPass(), windowDim);
}


void RenderCoordinator::destroy(){
    // teardown renderers
    m_frameRenderer.destroy();
    m_imguiRenderer.destroy();
    m_testRenderer.destroy();
    m_debugMeshRenderer.destroy();
    m_debugNormalsRenderer.destroy();
    m_depthPrepassRenderer.destroy();
    m_blurRenderer.destroy();
    m_gtaoRenderer.destroy();
    m_unlitMeshRenderer.destroy();
    m_litMeshRenderer.destroy();
}

}