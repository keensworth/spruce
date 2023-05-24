add_executable(assetcreate main.cpp)

target_sources(assetcreate PRIVATE 
  AssetCreator.h
  AssetCreator.cpp
)

target_include_directories(assetcreate PUBLIC ${CMAKE_CURRENT_LIST_DIR}/asset_create)

target_link_libraries(assetcreate glm tinygltf imgui stb_image)
target_compile_options(assetcreate PUBLIC)

