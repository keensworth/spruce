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

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated-volatile -Wno-unused-command-line-argument -Wno-nullability-completeness")

# project command is required to come after cmake_minimum_required command.
project(Spruce VERSION 0.0.1 LANGUAGES C CXX)


# ------------------------------------------------------------------
# choose build type
# ------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()
if (NOT SPRUCE_IS_SUBPROJECT)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
endif()

# set module path appropriately
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
if (SPRUCE_IS_SUBPROJECT)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



# ------------------------------------------------------------------
# third party
# ------------------------------------------------------------------
add_subdirectory(external)


# ------------------------------------------------------------------
# spruce library itself, including spruce-common
# ------------------------------------------------------------------
option(SPRUCE_BUILD_SHARED "Build SPRUCE as a shared library? (otherwise static)" OFF)
set(BUILD_SHARED_LIBS ${SPRUCE_BUILD_SHARED}) # use 'SPRUCE_' naming convention
add_subdirectory(src)
add_executable(spruce main.cpp)
target_link_libraries(spruce PUBLIC srcFiles)
target_compile_options(spruce PUBLIC -g -lSDL2 -lSDL -std=c++20 -I/usr/include/SDL2)


# ------------------------------------------------------------------
# some basic testing
# ------------------------------------------------------------------
option(SPRUCE_BUILD_ADVANCED_TESTS "Build the *advanced* test-cases?" OFF)
if (SPRUCE_BUILD_ADVANCED_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(tests)
endif()

# ------------------------------------------------------------------
# tools
# ------------------------------------------------------------------
add_subdirectory(tools)
