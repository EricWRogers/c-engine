cmake_minimum_required(VERSION 3.20)
project(c-engine)
set(ASSETS_DIR_NAME assets)
set(PROJECT_SETTINGS_DIR_NAME project_settings)

set(CANIS_PLATFORM_WEB OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(CANIS_PLATFORM_WEB ON)
endif()

set(CANIS_DEFAULT_EDITOR ON)
if(CANIS_PLATFORM_WEB)
  set(CANIS_DEFAULT_EDITOR OFF)
endif()
option(CANIS_ENABLE_EDITOR "Build editor support" ${CANIS_DEFAULT_EDITOR})

if(CANIS_PLATFORM_WEB AND CANIS_ENABLE_EDITOR)
  message(WARNING "Web builds do not support the editor runtime. Disabling CANIS_ENABLE_EDITOR.")
  set(CANIS_ENABLE_EDITOR OFF CACHE BOOL "Build editor support" FORCE)
endif()

set(CANIS_EDITOR_DEFINE 0)
if(CANIS_ENABLE_EDITOR)
  set(CANIS_EDITOR_DEFINE 1)
endif()

set(CANIS_DESKTOP_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/project")
set(CANIS_WEB_EXPORT_DIRECTORY "${CMAKE_BINARY_DIR}/web")

if(CANIS_PLATFORM_WEB)
  set(CANIS_RUNTIME_OUTPUT_DIRECTORY "${CANIS_WEB_EXPORT_DIRECTORY}")
else()
  set(CANIS_RUNTIME_OUTPUT_DIRECTORY "${CANIS_DESKTOP_RUNTIME_OUTPUT_DIRECTORY}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CANIS_RUNTIME_OUTPUT_DIRECTORY}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-fno-gnu-unique)
endif()

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
  message(STATUS "Using ccache: ${CCACHE_PROGRAM}")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  set(CMAKE_C_COMPILER_LAUNCHER  "${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache not found")
endif()

if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  find_program(MOLD_LINKER mold)
  find_program(LLD_LINKER  ld.lld)

  if (MOLD_LINKER)
    message(STATUS "Using mold linker: ${MOLD_LINKER}")
    add_link_options(-fuse-ld=mold)
  elseif (LLD_LINKER)
    message(STATUS "Using lld linker: ${LLD_LINKER}")
    add_link_options(-fuse-ld=lld)
  else()
    message(STATUS "No mold/lld found; using default linker")
  endif()
endif()

#if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
#  add_link_options(-fuse-ld=lld -Wl,--gc-sections)
#  add_compile_options(-ffunction-sections -fdata-sections)
#endif()
#string(APPEND CMAKE_CXX_FLAGS_DEBUG " -gsplit-dwarf")

set_property(GLOBAL PROPERTY CMAKE_CONFIGURE_DEPENDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

#set(SDLSHADERCROSS_VENDORED     ON)
#set(SDLSHADERCROSS_DXC          ON)
#set(SDLSHADERCROSS_DXC OFF CACHE BOOL "")
#set(SDL_GPU_ENABLE_SPIRV_CROSS  ON CACHE BOOL "")
#set(SDL_GPU_ENABLE_SHADERCROSS  ON CACHE BOOL "")
set(SDL_OPENGLES ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
if(CANIS_PLATFORM_WEB)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
endif()

set(glew-cmake_BUILD_SHARED         FALSE)
set(glew-cmake_BUILD_STATIC         TRUE)

if( NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
add_subdirectory(external/GL)
endif()

add_subdirectory(external/SDL)
add_subdirectory(external/stb)

# imgui
set(IMGUI_PATH  "external/imgui")
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp ${IMGUI_PATH}/backends/imgui_impl_sdl3.cpp ${IMGUI_PATH}/backends/imgui_impl_sdlgpu3.cpp ${IMGUI_PATH}/misc/cpp/imgui_stdlib.cpp)
file(GLOB IMGUI_HEADERS ${IMGUI_PATH}/*.h ${IMGUI_PATH}/backends/imgui_impl_opengl3.h ${IMGUI_PATH}/backends/imgui_impl_sdl3.h ${IMGUI_PATH}/backends/imgui_impl_sdlgpu3.h ${IMGUI_PATH}/misc/cpp/imgui_stdlib.h)
add_library("imgui" STATIC ${IMGUI_SOURCES})
target_include_directories("imgui" PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends/ ${IMGUI_PATH}/misc/cpp/)
target_link_libraries("imgui" PRIVATE SDL3::SDL3)

if(CANIS_ENABLE_EDITOR)
  # imguizmo
  set(IMGUIZMO_PATH "external/imguizmo")
  file(GLOB IMGUIZMO_SOURCES ${IMGUIZMO_PATH}/*.cpp)
  file(GLOB IMGUIZMO_HEADERS ${IMGUIZMO_PATH}/*.h)
  add_library("imguizmo" STATIC ${IMGUIZMO_SOURCES})
  target_include_directories("imguizmo" PUBLIC ${IMGUIZMO_PATH})
  target_link_libraries("imguizmo" PRIVATE "imgui" SDL3::SDL3)
endif()

# yaml-cpp
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Disable yaml-cpp tools" FORCE)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "Disable yaml-cpp tests" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Disable shared libs" FORCE)
add_subdirectory(external/yaml-cpp)

# glm
add_subdirectory(external/glm)

# Jolt Physics
set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "" FORCE)
set(OVERRIDE_CXX_FLAGS OFF CACHE BOOL "" FORCE)
set(INTERPROCEDURAL_OPTIMIZATION OFF CACHE BOOL "" FORCE)

if (MINGW)
  set(JPH_USE_DXC OFF CACHE BOOL "" FORCE)
else()
  set(JPH_USE_DXC ON CACHE BOOL "" FORCE)
endif()

set(JOLT_BUILD_DIR "${CMAKE_SOURCE_DIR}/external/JoltPhysics/Build")

if (EXISTS "${JOLT_BUILD_DIR}/CMakeLists.txt")
  add_subdirectory(external/JoltPhysics/Build EXCLUDE_FROM_ALL)
else()
  message(FATAL_ERROR
    "JoltPhysics submodule is not initialized. "
    "Run: git submodule update --init --recursive external/JoltPhysics")
endif()

# Jolt's CMake sets C++17 globally; restore project requirement.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(canis)
add_subdirectory(game)

if(CANIS_PLATFORM_WEB)
  target_link_libraries(${PROJECT_NAME} PRIVATE GameCode)
endif()
