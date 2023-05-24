add_executable(register_assets main.cpp)

target_sources(register_assets PRIVATE 
  AssetRegisterer.h
  AssetRegisterer.cpp
)

target_include_directories(register_assets PUBLIC ${CMAKE_CURRENT_LIST_DIR}/register_assets)

target_link_libraries(register_assets json glm srcFiles)
target_compile_options(register_assets PUBLIC)

