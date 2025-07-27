cmake_minimum_required(VERSION 3.20)
project(MyEngineProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SDLSHADERCROSS_VENDORED     ON)
#set(SDLSHADERCROSS_DXC          ON)
set(SDLSHADERCROSS_DXC OFF CACHE BOOL "")
set(SDL_GPU_ENABLE_SPIRV_CROSS  ON CACHE BOOL "")
set(SDL_GPU_ENABLE_SHADERCROSS  ON CACHE BOOL "")

add_subdirectory(external/SDL)
add_subdirectory(external/SDL_shadercross)
add_subdirectory(canis)
add_subdirectory(game)
