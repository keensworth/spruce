add_executable(gltfparser main.cpp)

target_sources(gltfparser PRIVATE 
  GLTFParser.h
  GLTFParser.cpp
)

target_include_directories(gltfparser PUBLIC ${CMAKE_CURRENT_LIST_DIR}/gltfparser)

target_link_libraries(gltfparser glm tinygltf imgui stb_image ktx)
target_compile_options(gltfparser PUBLIC)

