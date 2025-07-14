cmake_minimum_required(VERSION 3.18)
project(c-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/dist/${CMAKE_SYSTEM_NAME}>)
set(ASSETS_DIR_NAME assets)
set(ASSETS_RELEASE_DIR ${CMAKE_SOURCE_DIR}/assets/scripts/bin/Release/netstandard2.0)
set(CANIS_ENGINE_LIB CanisEngine)
set(ROOT_INCLUDE ${CMAKE_SOURCE_DIR}/src)
set(SCRIPTING_INCLUDE ${ROOT_INCLUDE}/scripting)

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

# Set Mono paths
set(MONO_ROOT "${CMAKE_SOURCE_DIR}/external/mono")
set(MONO_INCLUDE "${MONO_ROOT}/include/mono-2.0")
set(MONO_LIB "${MONO_ROOT}/lib")
set(MONO_ETC "${MONO_ROOT}/etc")
set(MONO_DLL "${MONO_ROOT}/mono-2.0-sgen.dll")

# add game library
file(GLOB_RECURSE CANIS_SOURCES src/*.c*)
file(GLOB_RECURSE CANIS_HEADERS src/*.h*)

add_library(${CANIS_ENGINE_LIB} SHARED ${CANIS_HEADERS} ${CANIS_SOURCES})
set_target_properties(${CANIS_ENGINE_LIB} PROPERTIES
    # ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    # RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
target_include_directories(${CANIS_ENGINE_LIB} PRIVATE ${MONO_INCLUDE} ${ROOT_INCLUDE} ${SCRIPTING_INCLUDE} glm libglew_static SDL3::SDL3)
target_link_libraries(${CANIS_ENGINE_LIB} PRIVATE glm libglew_static SDL3::SDL3 "${MONO_LIB}/mono-2.0-sgen.lib")
target_compile_definitions(${CANIS_ENGINE_LIB} PRIVATE CANISENGINE_EXPORTS)

#  add game executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_include_directories(${PROJECT_NAME}
    PRIVATE
        glm
        SDL3::SDL3
        ${ROOT_INCLUDE}
        ${SCRIPTING_INCLUDE}
        ${MONO_INCLUDE}
)
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glm
        libglew_static
        SDL3::SDL3
        ${CANIS_ENGINE_LIB}
        "${MONO_LIB}/mono-2.0-sgen.lib"
)

# generate script binding wrappers & compile C# scripts
add_custom_command(
    TARGET ${CANIS_ENGINE_LIB} PRE_BUILD
    COMMAND cd ${CMAKE_SOURCE_DIR} && py ./tools/generate-scripting-wrappers.py && bash ./BuildCSharp.sh
    COMMENT "Generating script binding wrappers"
)

# Copy assets directory - // todo we should not copy over the scripts
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

# Copy game dlls
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSETS_RELEASE_DIR}/GameBindings.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSETS_RELEASE_DIR}/GameScripts.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    COMMENT "Copying game dlls"
)

# Copy Mono runtime files (lib and etc)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/etc
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MONO_DLL}" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MONO_LIB}/mono-2.0-sgen.lib" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/lib
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${MONO_ETC}" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/etc
    COMMENT "Copying Mono runtime files for deployment"
)

# Copy Mono's class libraries (e.g., mscorlib.dll, System.dll, etc.)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/lib/mono/4.5
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${MONO_LIB}/mono/4.5" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mono/lib/mono/4.5
    COMMENT "Copying Mono 4.5 runtime assemblies"
)
