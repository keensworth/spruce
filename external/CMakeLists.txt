# find_package(Vulkan REQUIRED)

add_library(vkbootstrap STATIC)
add_library(glm INTERFACE)
add_library(vma INTERFACE)
add_library(stb_image INTERFACE)
add_library(tinyobjloader STATIC)
add_library(flathashmap INTERFACE)
add_library(json INTERFACE)
add_library(tinygltf STATIC)
add_library(oof INTERFACE)

if(UNIX)
  set(VOLK_STATIC_DEFINES "VK_USE_PLATFORM_XLIB_KHR")
elseif(WIN32)
  set(VOLK_STATIC_DEFINES "VK_USE_PLATFORM_WIN32_KHR")
endif()
add_subdirectory(volk)

target_sources(vkbootstrap PRIVATE 
  vkbootstrap/VkBootstrap.h
  vkbootstrap/VkBootstrap.cpp
  )

target_sources(tinygltf PRIVATE 
  tinygltf/tiny_gltf.h
  tinygltf/tiny_gltf.cpp
  )

target_include_directories(vkbootstrap PUBLIC vkbootstrap)
target_link_libraries(vkbootstrap PUBLIC volk $<$<BOOL:UNIX>:${CMAKE_DL_LIBS}>)

target_include_directories(flathashmap INTERFACE flathashmap)
target_include_directories(json INTERFACE json)
target_include_directories(oof INTERFACE oof)
target_include_directories(vma INTERFACE vma)
target_include_directories(tinygltf PUBLIC tinygltf)
target_link_libraries(tinygltf PUBLIC json stb_image)



#both vma and glm and header only libs so we only need the include path
# target_include_directories(vma INTERFACE vma)
target_include_directories(glm INTERFACE glm)
target_compile_options(glm INTERFACE)

target_sources(tinyobjloader PRIVATE 
  tinyobjloader/tiny_obj_loader.h
  tinyobjloader/tiny_obj_loader.cc
  )

target_include_directories(tinyobjloader PUBLIC tinyobjloader)

add_library(sdl2 INTERFACE)
set(sdl2_DIR "SDL_PATH" CACHE FILEPATH "/usr/include/SDL2/")

if (WIN32)
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include ${sdl2_image_DIR}/include)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib/x64 ${sdl2_image_DIR}/lib/x64)
elseif (UNIX)
target_include_directories(sdl2 INTERFACE ${sdl2_DIR}/include/SDL2 ${sdl2_image_DIR}/include/SDL2)
target_link_directories(sdl2 INTERFACE ${sdl2_DIR}/lib ${sdl2_image_DIR}/lib)
endif()

target_link_libraries(sdl2 INTERFACE SDL2 SDL2main)
target_compile_options(sdl2 INTERFACE -lSDL2 -I/usr/include/SDL2)

add_library(imgui SHARED)

target_include_directories(imgui PUBLIC imgui)

target_sources(imgui PRIVATE 
  imgui/imconfig.h
  imgui/imgui.cpp
  imgui/imgui.h
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_impl_sdl2.cpp
  imgui/imgui_impl_sdl2.h
  imgui/imgui_impl_vulkan.cpp
  imgui/imgui_impl_vulkan.h
  imgui/imgui_internal.h
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/imstb_rectpack.h
  imgui/imstb_textedit.h
  imgui/imstb_truetype.h
  )

target_link_libraries(imgui PUBLIC volk -lSDL2 sdl2)

target_include_directories(stb_image INTERFACE stb_image)
