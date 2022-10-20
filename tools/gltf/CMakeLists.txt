add_executable(gltfparser main.cpp)

target_include_directories(gltfparser PUBLIC gltfparser)

target_sources(gltfparser PRIVATE 
  GLTFParser.h
  GLTFParser.cpp
  FileWriter.h
)

target_link_libraries(gltfparser glm tinygltf imgui stb_image)
target_compile_options(gltfparser PUBLIC -fconcepts)

