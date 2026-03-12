set(CANIS_ENGINE_LIB CanisEngine)

set(CANISENGINE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")
message(STATUS "CANISENGINE_INCLUDE_DIRS ${CANISENGINE_INCLUDE_DIRS}")


file(GLOB_RECURSE CANIS_SOURCES src/*.c*)
file(GLOB_RECURSE CANIS_HEADERS include/Canis/*.h*)

add_library(${CANIS_ENGINE_LIB} SHARED ${CANIS_HEADERS} ${CANIS_SOURCES})
set_target_properties(${CANIS_ENGINE_LIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(${CANIS_ENGINE_LIB}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    $<TARGET_PROPERTY:yaml-cpp,INTERFACE_INCLUDE_DIRECTORIES>
)
target_link_libraries(${CANIS_ENGINE_LIB}
    PUBLIC
    glm
    imgui
    PRIVATE
    yaml-cpp
    SDL3::SDL3
    stb
    imguizmo
    libglew_static
    Jolt
)

find_package(Freetype QUIET)
if (Freetype_FOUND)
    target_link_libraries(${CANIS_ENGINE_LIB} PRIVATE Freetype::Freetype)
    target_compile_definitions(${CANIS_ENGINE_LIB} PRIVATE CANIS_HAS_FREETYPE)
    message(STATUS "CanisEngine: FreeType enabled")
else()
    message(WARNING "CanisEngine: FreeType not found. TextAsset loading will be disabled.")
endif()

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    glm
    yaml-cpp
    imgui
    ${CANIS_ENGINE_LIB}
    PRIVATE
    SDL3::SDL3
    stb
    libglew_static
    imguizmo
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_precompile_headers(${PROJECT_NAME}
    PUBLIC
    <string>
    <vector>
)

#add_custom_command(TARGET CanisEngine POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:${PROJECT_NAME}>
#        $<TARGET_FILE_DIR:CanisEngine>
#)
