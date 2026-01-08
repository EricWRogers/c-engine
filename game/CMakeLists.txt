set(GAME_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/game")
message(STATUS "GAME_INCLUDE_DIRS ${GAME_INCLUDE_DIRS}")

file(GLOB_RECURSE GAME_SOURCES src/*.c*)
file(GLOB_RECURSE GAME_HEADERS include/*.h*)

message(STATUS "GAME_INCLUDE_DIRS ${GAME_HEADERS}")

add_library(GameCode SHARED
    ${GAME_HEADERS}
    ${GAME_SOURCES}
)

# Include SDL3 headers if needed
target_include_directories(GameCode PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    CanisEngine
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
            ${CMAKE_SOURCE_DIR}/../${ASSETS_DIR_NAME}/
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSETS_DIR_NAME})
    add_custom_command(TARGET GameCode POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/${PROJECT_SETTINGS_DIR_NAME}/
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_SETTINGS_DIR_NAME})
else()
    add_custom_command(TARGET GameCode PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/${ASSETS_DIR_NAME}/
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSETS_DIR_NAME})
    add_custom_command(TARGET GameCode PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            ${CMAKE_SOURCE_DIR}/${PROJECT_SETTINGS_DIR_NAME}/
            ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_SETTINGS_DIR_NAME})
endif()