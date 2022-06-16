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


add_library(src)

#include(cmake/configure_tbb.cmake)

target_sources(src PRIVATE
  core/util/Container.h
  core/util/Container.cpp
  core/util/node/EntityNode.h
  core/util/node/EntityNode.cpp
  core/util/node/IndexNode.h
  core/util/node/IndexNode.cpp

  ecs/Registry.h
  ecs/Registry.cpp
  ecs/Entity.h
  ecs/Entity.cpp
  ecs/Component.h
  ecs/Component.cpp
  ecs/System.h
  ecs/EntityManager.h
  ecs/EntityManager.cpp
  ecs/ComponentManager.h
  ecs/ComponentManager.cpp
  ecs/SystemManager.h
  ecs/SystemManager.cpp
  ecs/SprECS.h
  ecs/SprECS.cpp



)

add_executable(spruce main.cpp)

#target_link_libraries(src PUBLIC external)
target_link_libraries(spruce PUBLIC src)


target_include_directories(src
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
