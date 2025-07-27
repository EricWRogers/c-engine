# Build the game logic as a shared library
add_library(GameCode SHARED
    GameCode.cpp
    GameCode.hpp
)

# Include SDL3 headers if needed
target_include_directories(GameCode PRIVATE
    ${SDL3_INCLUDE_DIRS}
)

# Link to SDL3 (for logging or future rendering use)
target_link_libraries(GameCode
    SDL3::SDL3
)

# Set output directory to match the engine executable
set_target_properties(GameCode PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
