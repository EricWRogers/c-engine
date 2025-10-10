cmake_minimum_required(VERSION 3.20)
project(c-engine)
set(ASSETS_DIR_NAME assets)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/dist/${CMAKE_SYSTEM_NAME}>)

set(CMAKE_CXX_STANDARD 20)

add_definitions(-DCANIS_EDITOR)

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
set(SDL_OPENGLES ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)

add_subdirectory(external/SDL)
add_subdirectory(external/stb)

# imgui
set(IMGUI_PATH  "external/imgui")
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp ${IMGUI_PATH}/backends/imgui_impl_sdl3.cpp ${IMGUI_PATH}/misc/cpp/imgui_stdlib.cpp) 
file(GLOB IMGUI_HEADERS ${IMGUI_PATH}/*.h ${IMGUI_PATH}/backends/imgui_impl_opengl3.h ${IMGUI_PATH}/backends/imgui_impl_sdl3.h ${IMGUI_PATH}/misc/cpp/imgui_stdlib.h) 
add_library("imgui" STATIC ${IMGUI_SOURCES})
target_include_directories("imgui" PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends/ ${IMGUI_PATH}/misc/cpp/)
target_link_libraries("imgui" PRIVATE SDL3::SDL3)

# imguizmo
set(IMGUIZMO_PATH "external/imguizmo")
file(GLOB IMGUIZMO_SOURCES ${IMGUIZMO_PATH}/*.cpp)
file(GLOB IMGUIZMO_HEADERS ${IMGUIZMO_PATH}/*.h)
add_library("imguizmo" STATIC ${IMGUIZMO_SOURCES})
target_include_directories("imguizmo" PUBLIC ${IMGUIZMO_PATH})
target_link_libraries("imguizmo" PRIVATE "imgui" SDL3::SDL3)

add_subdirectory(canis)
add_subdirectory(game)