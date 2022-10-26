add_executable(register_assets main.cpp)

target_include_directories(register_assets PUBLIC register_assets)

target_sources(register_assets PRIVATE 
  ../../src/resource/ResourceTypes.h
  AssetRegisterer.h
  AssetRegisterer.cpp
)

target_link_libraries(register_assets json glm srcFiles)
target_compile_options(register_assets PUBLIC)

