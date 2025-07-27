# Create the main engine executable
add_executable(EngineApp
    main.cpp
)

# Include SDL3 headers
target_include_directories(EngineApp PRIVATE
    ${SDL3_INCLUDE_DIRS}
)

# Link to SDL3 with GPU support
target_link_libraries(EngineApp
    SDL3::SDL3
)

# Set output path for easier library loading
set_target_properties(EngineApp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Ensure GameCode shared library is copied next to executable
add_custom_command(TARGET EngineApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:GameCode>
        $<TARGET_FILE_DIR:EngineApp>
)
