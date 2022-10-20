add_executable(register_assets main.cpp)

target_include_directories(register_assets PUBLIC register_assets)

target_sources(register_assets PRIVATE 
  GLTFParser.h
  GLTFParser.cpp
  FileWriter.h
)

target_link_libraries(register_assets glm tinygltf imgui stb_image)
target_compile_options(register_assets PUBLIC -fconcepts)

