set(GAME_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/game")
message(STATUS "GAME_INCLUDE_DIRS ${GAME_INCLUDE_DIRS}")

set(GENERATED_REGISTER_SCRIPTS_CPP "${CMAKE_CURRENT_SOURCE_DIR}/src/RegisterScripts.cpp")
set(GENERATE_REGISTER_SCRIPTS_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateRegisterScripts.cmake")

file(GLOB_RECURSE REGISTER_SCRIPT_HEADERS
    CONFIGURE_DEPENDS
    RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

set(REGISTER_SCRIPT_HEADER_PATHS "")
foreach(script_header IN LISTS REGISTER_SCRIPT_HEADERS)
    if (script_header STREQUAL "RegisterScripts.hpp")
        continue()
    endif()

    list(APPEND REGISTER_SCRIPT_HEADER_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/include/${script_header}")
endforeach()

list(SORT REGISTER_SCRIPT_HEADER_PATHS)
string(JOIN "||" REGISTER_SCRIPT_HEADER_PATHS_ARG ${REGISTER_SCRIPT_HEADER_PATHS})

add_custom_command(
    OUTPUT "${GENERATED_REGISTER_SCRIPTS_CPP}"
    COMMAND ${CMAKE_COMMAND}
        -DOUTPUT_FILE=${GENERATED_REGISTER_SCRIPTS_CPP}
        -DINCLUDE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}/include
        -DHEADER_PATHS=${REGISTER_SCRIPT_HEADER_PATHS_ARG}
        -P ${GENERATE_REGISTER_SCRIPTS_SCRIPT}
    DEPENDS
        "${GENERATE_REGISTER_SCRIPTS_SCRIPT}"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/RegisterScripts.hpp"
        ${REGISTER_SCRIPT_HEADER_PATHS}
    VERBATIM
)

set_source_files_properties("${GENERATED_REGISTER_SCRIPTS_CPP}" PROPERTIES GENERATED TRUE)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS src/*.c*)
file(GLOB_RECURSE GAME_HEADERS CONFIGURE_DEPENDS include/*.h*)

list(FILTER GAME_SOURCES EXCLUDE REGEX ".*/RegisterScripts\\.cpp$")

message(STATUS "GAME_INCLUDE_DIRS ${GAME_HEADERS}")

add_library(GameCode SHARED
    ${GAME_HEADERS}
    ${GAME_SOURCES}
    ${GENERATED_REGISTER_SCRIPTS_CPP}
)

# Include SDL3 headers if needed
target_include_directories(GameCode PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_precompile_headers(GameCode PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/include/GamePCH.hpp>"
)

# Link to SDL3 (for logging or future rendering use)
target_link_libraries(GameCode
    CanisEngine
)

# Set output directory to match the engine executable
set_target_properties(GameCode PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

# Copy assets directory
if (WIN32)
    add_custom_command(TARGET GameCode POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/${PROJECT_SETTINGS_DIR_NAME}/
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_SETTINGS_DIR_NAME})
else()
    add_custom_command(TARGET GameCode PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/${PROJECT_SETTINGS_DIR_NAME}/
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_SETTINGS_DIR_NAME})
endif()
