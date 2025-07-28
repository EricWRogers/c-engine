cmake_minimum_required(VERSION 3.20)
project(c-engine)
set(ASSETS_DIR_NAME assets)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/dist/${CMAKE_SYSTEM_NAME}>)

set(CMAKE_CXX_STANDARD 20)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message(STATUS "Found ccache at ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache not found")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#set(SDLSHADERCROSS_VENDORED     ON)
#set(SDLSHADERCROSS_DXC          ON)
#set(SDLSHADERCROSS_DXC OFF CACHE BOOL "")
#set(SDL_GPU_ENABLE_SPIRV_CROSS  ON CACHE BOOL "")
#set(SDL_GPU_ENABLE_SHADERCROSS  ON CACHE BOOL "")

add_subdirectory(external/SDL)
#add_subdirectory(external/SDL_shadercross)
add_subdirectory(canis)
add_subdirectory(game)