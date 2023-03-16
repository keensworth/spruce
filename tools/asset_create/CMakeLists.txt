add_executable(assetcreate main.cpp)

target_include_directories(assetcreate PUBLIC assetcreate)

target_sources(assetcreate PRIVATE 
  AssetCreator.h
  AssetCreator.cpp
)

target_link_libraries(assetcreate glm tinygltf imgui stb_image)
target_compile_options(assetcreate PUBLIC)

