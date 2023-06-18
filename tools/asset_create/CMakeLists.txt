add_executable(assetcreate main.cpp)

target_sources(assetcreate PRIVATE 
  AssetCreator.h
  AssetCreator.cpp
)

target_include_directories(assetcreate PUBLIC ${CMAKE_CURRENT_LIST_DIR}/asset_create)

target_link_libraries(assetcreate glm imgui stb_image ktx)
target_compile_options(assetcreate PUBLIC)

