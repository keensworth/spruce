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

cmake_minimum_required(VERSION 3.12)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.18)
  cmake_policy(SET CMP0104 NEW)
endif()
cmake_policy(SET CMP0076 NEW)

if (NOT (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR}))
  set(SPRUCE_IS_SUBPROJECT ON)
else()
  set(SPRUCE_IS_SUBPROJECT OFF)
endif()

set(CMAKE_BUILD_TYPE_INIT "Release")

# project command is required to come after cmake_minimum_required command.
project(Spruce VERSION 0.0.1 LANGUAGES C CXX)

#set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()
if (NOT SPRUCE_IS_SUBPROJECT)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
endif()

# ------------------------------------------------------------------
# first, include gdt project to do some general configuration stuff
# (build modes, glut, optix, etc)
# ------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
if (SPRUCE_IS_SUBPROJECT)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)
endif()
#include(configure_tbb)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------
# spruce library itself, including spruce-common
# ------------------------------------------------------------------
option(SPRUCE_BUILD_SHARED "Build SPRUCE as a shared library? (otherwise static)" OFF)
set(BUILD_SHARED_LIBS ${SPRUCE_BUILD_SHARED}) # use 'SPRUCE_' naming convention

#include_directories(${CMAKE_CURRENT_LIST_DIR}) 
add_subdirectory(src)
add_executable(spruce main.cpp)
#target_include_directories(spruce PRIVATE
#  ${CMAKE_SOURCE_DIR}/src
#)
target_link_libraries(spruce PUBLIC srcFiles)
target_compile_options(spruce PUBLIC -fconcepts -lSDL2 -lSDL)



option(SPRUCE_BUILD_ADVANCED_TESTS "Build the *advanced* test-cases?" OFF)

# ------------------------------------------------------------------
# third party
# ------------------------------------------------------------------
add_subdirectory(external)



if (SPRUCE_IS_SUBPROJECT)
  return()
endif()

include(CTest)
if (BUILD_TESTING)
  enable_testing()
endif()
# ------------------------------------------------------------------
# some basic testing
# ------------------------------------------------------------------
if (SPRUCE_BUILD_ADVANCED_TESTS)
  add_subdirectory(tests)
endif()
