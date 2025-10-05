set(CANIS_ENGINE_LIB CanisEngine)

set(CANISENGINE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")
message(STATUS "CANISENGINE_INCLUDE_DIRS ${CANISENGINE_INCLUDE_DIRS}")

file(GLOB_RECURSE CANIS_SOURCES src/*.c*)
file(GLOB_RECURSE CANIS_HEADERS include/Canis/*.h*)

add_library(${CANIS_ENGINE_LIB} SHARED ${CANIS_HEADERS} ${CANIS_SOURCES})
set_target_properties(${CANIS_ENGINE_LIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_include_directories(${CANIS_ENGINE_LIB} PUBLIC SDL3::SDL3 stb ${CMAKE_CURRENT_SOURCE_DIR}/include imgui imguizmo)
target_link_libraries(${CANIS_ENGINE_LIB} PUBLIC SDL3::SDL3 stb imgui imguizmo)

add_executable(${PROJECT_NAME} main.cpp)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    SDL3::SDL3
    stb
    GLESv2
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    imgui
    imguizmo
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    SDL3::SDL3
    stb
    GLESv2
    ${CANIS_ENGINE_LIB}
    imgui
    imguizmo
)

#add_custom_command(TARGET CanisEngine POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:${PROJECT_NAME}>
#        $<TARGET_FILE_DIR:CanisEngine>
#)
