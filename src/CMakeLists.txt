

add_library(srcFiles)


target_sources(srcFiles PRIVATE
  core/spruce_core.h
  core/memory/Handle.h
  core/memory/Pool.h
  core/memory/TempBuffer.h
  core/util/Container.h
  core/util/FunctionStack.h
  core/util/FunctionQueue.h
  core/util/node/EntityNode.h
  core/util/node/EntityNode.cpp
  core/util/node/IndexNode.h
  core/util/node/IndexNode.cpp

  debug/SprLog.cpp
  debug/SprLog.h

  ecs/Registry.h
  ecs/Registry.cpp
  ecs/Entity.h
  ecs/Entity.cpp
  ecs/Component.h
  ecs/System.h
  ecs/EntityManager.h
  ecs/EntityManager.cpp
  ecs/ComponentManager.h
  ecs/ComponentManager.cpp
  ecs/SystemManager.h
  ecs/SystemManager.cpp
  ecs/SprECS.h

  interface/InputHandler.h
  interface/InputHandler.cpp
  interface/InputManager.h
  interface/InputManager.cpp
  interface/KeyboardConfig.h
  interface/KeyboardConfig.cpp
  interface/MouseConfig.h
  interface/MouseConfig.cpp
  interface/KeyboardState.h
  interface/MouseState.h
  interface/Window.h
  interface/Window.cpp

  resource/ResourceTypes.h
  resource/AssetLoader.h
  resource/AssetLoader.cpp
  resource/ResourceCache.h
  resource/ResourceLoader.h
  resource/ResourceLoader.cpp
  resource/SprResourceManager.h
  resource/SprResourceManager.cpp

  render/vulkan/gfx_vulkan_core.h
  render/vulkan/VulkanDevice.h
  render/vulkan/VulkanDevice.cpp
  render/vulkan/VulkanDisplay.h
  render/vulkan/VulkanDisplay.cpp
  render/vulkan/CommandBuffer.h
  render/vulkan/CommandBuffer.cpp
  render/vulkan/CommandPool.h
  render/vulkan/CommandPool.cpp
  render/vulkan/resource/ResourceTypes.h
  render/vulkan/resource/ResourceFlags.h
  render/vulkan/resource/VulkanResourceCache.h
  render/vulkan/resource/VulkanResourceManager.cpp
  render/vulkan/resource/VulkanResourceManager.h
  render/vulkan/RenderFrame.h
  render/vulkan/VulkanRenderer.h
  render/vulkan/VulkanRenderer.cpp
  render/vulkan/StagingBufferBatch.h
  render/vulkan/StagingBufferBatch.cpp
  render/vulkan/GPUStreamer.h
  render/vulkan/GPUStreamer.cpp
  render/vulkan/UploadHandler.h
  render/vulkan/UploadHandler.cpp
  render/vulkan/DescriptorSetHandler.cpp
  render/vulkan/DescriptorSetHandler.h
  render/vulkan/RenderPassRenderer.h
  render/vulkan/RenderPassRenderer.cpp
  render/vulkan/FrameRenderer.h
  render/scene/Material.h
  render/scene/GfxAssetLoader.cpp
  render/scene/Draw.h
  render/scene/BatchNode.cpp
  render/scene/GfxAssetLoader.h
  render/scene/BatchNode.h
  render/scene/SceneData.h
  render/scene/BatchManager.h
  render/scene/BatchManager.cpp
  render/scene/Mesh.h
  render/SceneManager.h
  render/SceneManager.cpp
  render/RenderCoordinator.h
  render/RenderCoordinator.cpp
  render/SprRenderer.h
  render/SprRenderer.cpp
)


target_include_directories(srcFiles 
  PUBLIC ecs
  PUBLIC core
  PUBLIC debug
  PUBLIC interface
  PUBLIC resource
  PUBLIC render
  PUBLIC external
)

target_link_libraries(srcFiles PUBLIC vkbootstrap vma glm volk tinyobjloader imgui stb_image oof)
target_link_libraries(srcFiles PUBLIC  sdl2)
target_compile_options(srcFiles PUBLIC)



