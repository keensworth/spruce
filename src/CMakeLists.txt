# ======================================================================== #
# Copyright 2019-2021 Voxoft                                               #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ======================================================================== #

find_package(Vulkan REQUIRED)
add_library(srcFiles)

#include(cmake/configure_tbb.cmake)

target_sources(srcFiles PRIVATE
  core/Core.h
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

#target_link_libraries(src PUBLIC external)



target_include_directories(srcFiles 
  PUBLIC ecs
  PUBLIC core
  PUBLIC interface
)


target_link_libraries(srcFiles vkbootstrap vma glm tinyobjloader imgui stb_image)
target_link_libraries(srcFiles Vulkan::Vulkan sdl2)
target_compile_options(srcFiles PUBLIC -fconcepts)



