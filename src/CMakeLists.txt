
find_package(Vulkan REQUIRED)
add_library(srcFiles)


target_sources(srcFiles PRIVATE
  core/spruce_core.h
  core/memory/Handle.h
  core/memory/Pool.h
  core/util/Container.h
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
  resource/ResourceManager.h
  resource/ResourceManager.cpp

  # render/util/BatchManager.cpp
  # render/util/BatchManager.h
  # render/RenderCoordinator.h
  # render/SprRenderer.h
  # render/util/BatchNode.cpp
  # render/util/BatchNode.h
  # render/vulkan/CommandBuffer.h
  # render/vulkan/PipelineManager.h
  # render/vulkan/RenderPassRenderer.h
  # render/vulkan/SwapChain.h
  # render/vulkan/VulkanDevice.cpp
  # render/vulkan/VulkanDevice.h
  # render/vulkan/VulkanDisplay.h
  # render/vulkan/VulkanRenderer.h
  # render/vulkan/resource/ResourceTypes.h
  # render/vulkan/resource/VulkanResourceCache.h
  # render/vulkan/resource/VulkanResourceManager.cpp
  # render/vulkan/resource/VulkanResourceManager.h
  # render/vulkan/vulkan_core.h
)



target_include_directories(srcFiles 
  PUBLIC ecs
  PUBLIC core
  PUBLIC debug
  PUBLIC interface
  PUBLIC resource
)


target_link_libraries(srcFiles vkbootstrap vma glm tinyobjloader imgui stb_image oof)
target_link_libraries(srcFiles Vulkan::Vulkan sdl2)
target_compile_options(srcFiles PUBLIC)



