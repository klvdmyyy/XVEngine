set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
cmake_minimum_required(VERSION 4.1.1)

include(shaders)
add_shaders(simple_shaders
  shaders/simple_shader.vert
  shaders/simple_shader.frag)

project(Game
  LANGUAGES CXX
  VERSION 1.0.0)

set(ENGINE_NAME XVEngine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(game)
file(GLOB_RECURSE GAME_SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

target_sources(game PRIVATE ${GAME_SOURCE_FILES})

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)
add_subdirectory(external/vk-bootstrap)

target_link_libraries(game PRIVATE SDL3::SDL3 Vulkan::Vulkan vk-bootstrap::vk-bootstrap)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/source/config.h.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/source/config.h"
  @ONLY
)
