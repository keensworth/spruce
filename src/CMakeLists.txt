# ======================================================================== #
# Copyright 2019-2021 Voxoft                                               #
#                                                                          #
# Licensed under the Apache License, Version 2.0 (the "License");          #
# you may not use this file except in compliance with the License.         #
# You may obtain a copy of the License at                                  #
#                                                                          #
#     http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                          #
# Unless required by applicable law or agreed to in writing, software      #
# distributed under the License is distributed on an "AS IS" BASIS,        #
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. #
# See the License for the specific language governing permissions and      #
# limitations under the License.                                           #
# ======================================================================== #


add_library(src)

#include(cmake/configure_tbb.cmake)

target_sources(src PRIVATE
  #file_names_here.cpp oh fuck
)

add_executable(src hostCode.cpp)

target_link_libraries(src PUBLIC external)

target_include_directories(src
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
