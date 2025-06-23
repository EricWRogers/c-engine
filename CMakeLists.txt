cmake_minimum_required(VERSION 3.18)
project(c-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/dist/${CMAKE_SYSTEM_NAME}>)
set(ASSETS_DIR_NAME assets)
set(CANIS_ENGINE_LIB CanisEngine)

set(glew-cmake_BUILD_SHARED FALSE)
set(glew-cmake_BUILD_STATIC TRUE)

add_subdirectory(external/GL)
set_target_properties(libglew_static PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_subdirectory(external/glm)
add_subdirectory(external/SDL)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    message(STATUS "Found ccache at ${CCACHE_PROGRAM}")
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache not found")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(MONO REQUIRED mono-2)

file(GLOB_RECURSE CANIS_SOURCES src/*.c*)
file(GLOB_RECURSE CANIS_HEADERS src/*.h*)

add_library(${CANIS_ENGINE_LIB} SHARED ${CANIS_HEADERS} ${CANIS_SOURCES})
set_target_properties(${CANIS_ENGINE_LIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_include_directories(${CANIS_ENGINE_LIB} PRIVATE ${MONO_INCLUDE_DIRS} glm libglew_static SDL3::SDL3)
target_link_libraries(${CANIS_ENGINE_LIB} PRIVATE ${MONO_LIBRARIES} glm libglew_static SDL3::SDL3)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glm
        libglew_static
        SDL3::SDL3
        ${MONO_LIBRARIES}
        ${CANIS_ENGINE_LIB}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        glm
        SDL3::SDL3
        ${MONO_INCLUDE_DIRS}
)

# Copy assets directory
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/${ASSETS_DIR_NAME}/
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSETS_DIR_NAME})
else()
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/${ASSETS_DIR_NAME}/
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSETS_DIR_NAME})
endif()

# Compile C# scripts
file(GLOB_RECURSE CS_SCRIPTS ${CMAKE_SOURCE_DIR}/assets/scripts/*.cs)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND mcs -target:library -out:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/GameScripts.dll ${CS_SCRIPTS}
    COMMENT "Recompiling all C# scripts every build"
)

# Copy Mono runtime files (lib and etc)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/etc
    COMMAND ${CMAKE_COMMAND} -E copy_if_different /usr/lib/libmonosgen-2.0.so ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/lib
    COMMAND ${CMAKE_COMMAND} -E copy_directory /etc/mono ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/etc
    COMMENT "Copying Mono runtime files for deployment"
)

# Copy Mono's class libraries (e.g., mscorlib.dll, System.dll, etc.)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/lib/mono/4.5
    COMMAND ${CMAKE_COMMAND} -E copy_directory /usr/lib/mono/4.5 ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/lib/mono/4.5
    COMMENT "Copying Mono 4.5 runtime assemblies"
)
