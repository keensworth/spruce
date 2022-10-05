add_executable(gltfparser main.cpp)

target_link_libraries(gltfparser glm tinygltf imgui stb_image)
target_compile_options(gltfparser PUBLIC -fconcepts)



