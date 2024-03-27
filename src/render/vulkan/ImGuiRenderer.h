#pragma once

#include "VulkanRenderer.h"
#include "glm/geometric.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "resource/ResourceTypes.h"
#include "resource/VulkanResourceManager.h"
#include "render/scene/BatchManager.h"
#include "resource/ResourceFlags.h"
#include "debug/SprLog.h"
#include "../external/imgui/imgui_impl_sdl2.h"
#include "../external/imgui/imgui_impl_vulkan.h"
#include "interface/SprWindow.h"
#include "scene/SceneData.h"



namespace spr::gfx {

struct RenderState {
    enum Shader : uint32 {
        TEST = 0b1,
        DEBUG_MESH = 0b1 << 1,
        UNLIT_MESH = 0b1 << 2,
        LIT_MESH = 0b1 << 3,
        DEBUG_NORMALS = 0b1 << 4,
        DEPTH_PREPASS = 0b1 << 5,
        GTAO_PASS = 0b1 << 6,
        BLUR_PASS = 0b1 << 7,
        SHADOW_CASCADES = 0b1 << 8,
        VOLUMETRIC_LIGHT = 0b1 << 9,
        FXAA = 0b1 << 10
    };

    Shader visible = FXAA;
    bool dirtyOutput = false;
    bool dirtyShader = false;
    Shader shaderToReload = TEST;
    bool dirtyLight = false;

    uint32 shadowSelection = 0;
    float cascadeLambda = 0.92f;
    float exposure = 5.f;
    glm::vec3 lightColor = {1.f, 1.f, 1.f};
    glm::vec3 lightDir = glm::normalize(vec3(0.3f, 1.f, -2.f));

    uint32 windowDim = 1;
    uint32 windowMode = 3;
    bool dirtyWindow = false;
};

class ImGuiRenderer {
public:
    ImGuiRenderer(){}
    ImGuiRenderer(VulkanResourceManager& rm, VulkanRenderer& renderer, SprWindow* window, glm::uvec3 dimensions){
        m_rm = &rm;
        m_renderer = &renderer;
        m_dim = dimensions;
        m_window = window;

        VkDescriptorPoolSize pool_sizes[] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo pool_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = NULL,
            .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
            .maxSets = 1000,
            .poolSizeCount = std::size(pool_sizes),
            .pPoolSizes = pool_sizes
        };

        VK_CHECK(vkCreateDescriptorPool(m_renderer->getDevice().getDevice(), &pool_info, nullptr, &m_imguiDescriptorPool));
    }
    ~ImGuiRenderer(){}

private:
    void initImGui(){
        // -- imgui init --
        // create context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsClassic();

        // enable IO
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // init for SDL2
        ImGui_ImplSDL2_InitForVulkan(m_window->getHandle());

        // init for Vulkan
        VulkanDevice& device = m_renderer->getDevice();
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.getPhysicalDevice();
        init_info.Device = device.getDevice();
        init_info.QueueFamily = device.getQueueFamilies().graphicsFamilyIndex.value();
        init_info.Queue = device.getQueue(VulkanDevice::QueueType::GRAPHICS);
        init_info.DescriptorPool = m_imguiDescriptorPool;
        init_info.MinImageCount = m_renderer->getDisplay().getImageViews().size();
        init_info.ImageCount = m_renderer->getDisplay().getImageViews().size();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        // give imgui its own function loader, we use volk elsewhere
        ImGui_ImplVulkan_LoadFunctions([](const char *function_name, void *vulkan_instance) {
            return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance *>(vulkan_instance)), function_name);
        }, &m_renderer->getDevice().getInstance());

        RenderPass* renderPass = m_rm->get<RenderPass>(m_renderPass);
        ImGui_ImplVulkan_Init(&init_info, renderPass->renderPass);

        // hook imgui's event listener into the Window
        m_window->addEventListener([=](SDL_Event* e) {
            ImGui_ImplSDL2_ProcessEvent(e);
        });
    }

    void buildLayout(){
        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->Size.x-370, main_viewport->WorkPos.y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(370, 680), ImGuiCond_Always);

        if (!ImGui::Begin("spruce")){
            ImGui::End();
            return;
        }

        // window settings
        if (!ImGui::CollapsingHeader("Window", ImGuiTreeNodeFlags_DefaultOpen)){
            ImGui::End();
            return;
        }
        const char* resolutions[] = { "2560x1440", "1920x1080", "1366x768", "1280x720" };
        static int currentRes = 1;
        ImGui::Combo("Resolution", &currentRes, resolutions, IM_ARRAYSIZE(resolutions));
        if (currentRes != state.windowDim){
            state.windowDim = currentRes;
            if (currentRes == 0){
                m_window->setResolution(2560, 1440);
            } else if (currentRes == 1){
                m_window->setResolution(1920, 1080);
            } else if (currentRes == 2){
                m_window->setResolution(1366, 768);
            } else {
                m_window->setResolution(1280, 720);
            }
        }

        const char* displayModes[] = { "Fullscreen", "Fullscreen Windowed", "Windowed Borderless", "Windowed" };
        static int currentMode = 3;
        ImGui::Combo("Display Mode", &currentMode, displayModes, IM_ARRAYSIZE(displayModes));
        if (currentMode != state.windowMode){
            state.windowMode = currentMode;
            if (currentMode == 0){
                m_window->setFullscreen();
            } else if (currentMode == 1){
                m_window->setFullscreen();
                m_window->setBorderless();
            } else if (currentMode == 2){
                m_window->setWindowed();
                m_window->setBorderless();
            } else {
                m_window->setWindowed();
                m_window->setBordered();
            }
        }

        // scene settings
        if (!ImGui::CollapsingHeader("Scene", ImGuiTreeNodeFlags_DefaultOpen)){
            ImGui::End();
            return;
        }
        ImGui::SeparatorText("Sun");
        state.dirtyLight |= ImGui::SliderFloat3("light dir", glm::value_ptr(state.lightDir), -1.0f, 1.0f);
        state.dirtyLight |= ImGui::ColorEdit3("light color", glm::value_ptr(state.lightColor));
        ImGui::SeparatorText("Shadow");
        ImGui::SliderFloat("cascade lambda", &state.cascadeLambda, 0.0f, 1.0f, "lambda = %.3f");
        ImGui::SeparatorText("Skybox");
        ImGui::SliderFloat("exposure", &state.exposure, 0.0f, 10.f, "exposure = %.3f");

        if (!ImGui::CollapsingHeader("Renderer", ImGuiTreeNodeFlags_DefaultOpen)){
            ImGui::End();
            return;
        }

        static int visible = RenderState::FXAA;
        //bool reload = ImGui::Button("Reload Shader");
        static int cascade = 0;

        if (ImGui::TreeNode("Debug View")){
            ImGui::RadioButton("Lit Mesh", &visible, RenderState::FXAA);
            ImGui::RadioButton("Unlit Mesh", &visible, RenderState::UNLIT_MESH);
            ImGui::RadioButton("Depth Prepass", &visible, RenderState::DEPTH_PREPASS);
            ImGui::RadioButton("GTAO Pass", &visible, RenderState::GTAO_PASS);
            ImGui::RadioButton("Blur Pass", &visible, RenderState::BLUR_PASS);
            ImGui::RadioButton("Cascaded Shadows", &visible, RenderState::SHADOW_CASCADES);
            if (visible & RenderState::SHADOW_CASCADES){
                ImGui::SliderInt("cascade #", &cascade, 0, MAX_CASCADES-1);
            }
            ImGui::RadioButton("Volumetric Light", &visible, RenderState::VOLUMETRIC_LIGHT);
            ImGui::RadioButton("Debug Mesh", &visible, RenderState::DEBUG_MESH);
            ImGui::RadioButton("Debug Normals", &visible, RenderState::DEBUG_NORMALS);
            ImGui::RadioButton("Test", &visible, RenderState::TEST);
        
            if ((uint32)visible != (uint32)state.visible){
                state.visible = (RenderState::Shader)visible;
                state.dirtyOutput = true;
            }
            if ((uint32)cascade != state.shadowSelection){
                state.shadowSelection = cascade;
                state.dirtyOutput = true;
            }

            ImGui::TreePop();
        }

        // shader settings
        if (ImGui::TreeNode("Shaders")){
            static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

            if (ImGui::BeginTable("table1", 3, flags)){
                ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Path (.frag/.vert)", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Reload", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("PBR Mesh");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/pbr_mesh");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##1")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::LIT_MESH;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Unlit Mesh");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/unlit_mesh");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##2")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::UNLIT_MESH;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Depth Prepass");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/depth_pass");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##3")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::DEPTH_PREPASS;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("GTAO");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/gtao");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##4")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::GTAO_PASS;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Blur");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/blur");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##5")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::BLUR_PASS;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Shadow Cascades");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/shadow_cascades");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##6")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::SHADOW_CASCADES;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Volumetric Light");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/volumetric_light");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##7")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::VOLUMETRIC_LIGHT;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("FXAA");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/fxaa");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##8")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::FXAA;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Debug Mesh");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/debug_mesh");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##9")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::DEBUG_MESH;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Debug Normals");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/debug_normals");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##10")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::DEBUG_NORMALS;
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Test");
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("data/shaders/test");
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Reload##11")){
                    state.dirtyShader = true;
                    state.shaderToReload = RenderState::TEST;
                }

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }

        ImGui::End();
    }

public:


    void init(
        Handle<DescriptorSet> globalDescriptorSet, 
        Handle<DescriptorSetLayout> globalDescSetLayout)
    {
        m_globalDescriptorSet = globalDescriptorSet;
        m_globalDescriptorSetLayout = globalDescSetLayout;

        // color attachment
        m_attachment = m_rm ->create<TextureAttachment>({
            .textureLayout = {
                .dimensions = m_dim,
                .format = Flags::Format::RGBA8_UNORM,
                .usage = Flags::ImageUsage::IU_COLOR_ATTACHMENT |
                         Flags::ImageUsage::IU_SAMPLED          
            }
        });

        // render pass
        m_renderPassLayout = m_rm->create<RenderPassLayout>({
            .colorAttatchmentFormats = {Flags::RGBA8_UNORM},
            .subpass = {
                .colorAttachments = {0}
            }
        });
        m_renderPass = m_rm->create<RenderPass>({
            .dimensions = m_dim,
            .layout = m_renderPassLayout,
            .colorAttachments = {
                {
                    .texture = m_attachment,
                    .finalLayout = Flags::ImageLayout::READ_ONLY
                }
            }
        });

        // descriptor set layout
        m_descriptorSetLayout = m_rm->create<DescriptorSetLayout>({
            .textures = {
                {.binding = 0}
            }
        });

        // shader
        m_shader = m_rm->create<Shader>({
            .vertexShader   = {.path = "../data/shaders/spv/copy.vert.spv"},
            .fragmentShader = {.path = "../data/shaders/spv/copy.frag.spv"},
            .descriptorSets = {
                { m_globalDescriptorSetLayout },
                { }, // unused
                { m_descriptorSetLayout },
                { }  // unused
            },
            .graphicsState = {
                .renderPass = m_renderPass,
            }
        });

        initImGui();
    }


    void setInput(Handle<TextureAttachment> input){
        if (input == m_input)
            return;

        m_rm->remove<DescriptorSet>(m_descriptorSet);

        m_descriptorSet = m_rm->create<DescriptorSet>({
            .textures = {
                {
                    .attachment = input,
                    .layout = Flags::ImageLayout::READ_ONLY
                }
            },
            .layout = m_descriptorSetLayout
        });

        m_hasInput = true;
    }


    void render(CommandBuffer& cb, BatchManager& batchManager){
        if (!m_hasInput)
            SprLog::error("[ImGuiRenderer] [render] no input TextureAttachment specified");
        
        if (!m_imguiInit) {
            ImGui_ImplVulkan_CreateFontsTexture(cb.getCommandBuffer());
            m_imguiInit = true;
        }

        // new frame
        ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame(m_window->getHandle());
		ImGui::NewFrame();
        
        // build out layout
        buildLayout();
        
        RenderPassRenderer& passRenderer = cb.beginRenderPass(m_renderPass, glm::vec4(0.f,1.f,0.f,1.f));
        
        ImGui::Render();        
        
        passRenderer.drawSubpass({
            .shader = m_shader,
            .set0 =  m_globalDescriptorSet,
            .set2 = m_descriptorSet}, 
            batchManager.getQuadBatch(), 0, 0);
        
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb.getCommandBuffer());
        
        cb.endRenderPass();
    }


    Handle<RenderPass> getRenderPass(){
        return m_renderPass;
    }
    
    Handle<TextureAttachment> getAttachment(){
        return m_attachment;
    }

    void destroy(){
        ImGui_ImplVulkan_DestroyFontUploadObjects();
        vkDestroyDescriptorPool(m_renderer->getDevice().getDevice(), m_imguiDescriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        m_rm->remove<DescriptorSet>(m_descriptorSet);
        m_rm->remove<Shader>(m_shader);
        m_rm->remove<DescriptorSetLayout>(m_descriptorSetLayout);
        m_rm->remove<RenderPass>(m_renderPass);
        m_rm->remove<RenderPassLayout>(m_renderPassLayout);
        m_rm->remove<TextureAttachment>(m_attachment);
        SprLog::info("[ImGuiRenderer] [destroy] destroyed...");
    }

    RenderState state;

private: // owning
    Handle<RenderPassLayout> m_renderPassLayout;
    Handle<RenderPass> m_renderPass;
    Handle<DescriptorSetLayout> m_descriptorSetLayout;
    Handle<DescriptorSet> m_descriptorSet;
    Handle<Shader> m_shader;
    Handle<TextureAttachment> m_attachment;
    VkDescriptorPool m_imguiDescriptorPool;

private: // non-owning
    VulkanResourceManager* m_rm;
    VulkanRenderer* m_renderer;
    SprWindow* m_window;
    glm::uvec3 m_dim;

    Handle<TextureAttachment> m_input;
    bool m_hasInput = false;
    bool m_imguiInit = false;

    Handle<DescriptorSet> m_globalDescriptorSet;
    Handle<DescriptorSetLayout> m_globalDescriptorSetLayout;
};
}