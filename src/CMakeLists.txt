
find_package(Vulkan REQUIRED)
add_library(srcFiles)


target_sources(srcFiles PRIVATE
  core/Core.h
  core/memory/Handle.h
  core/memory/Pool.h
  core/util/Container.h
  core/util/node/EntityNode.h
  core/util/node/EntityNode.cpp
  core/util/node/IndexNode.h
  core/util/node/IndexNode.cpp

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
)



target_include_directories(srcFiles 
  PUBLIC ecs
  PUBLIC core
  PUBLIC interface
)


target_link_libraries(srcFiles vkbootstrap vma glm tinyobjloader imgui stb_image)
target_link_libraries(srcFiles Vulkan::Vulkan sdl2)
target_compile_options(srcFiles PUBLIC)



