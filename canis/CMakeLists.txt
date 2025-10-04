set(CANIS_ENGINE_LIB CanisEngine)

set(CANISENGINE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")
message(STATUS "CANISENGINE_INCLUDE_DIRS ${CANISENGINE_INCLUDE_DIRS}")

file(GLOB_RECURSE CANIS_SOURCES src/*.c*)
file(GLOB_RECURSE CANIS_HEADERS include/Canis/*.h*)

add_library(${CANIS_ENGINE_LIB} SHARED ${CANIS_HEADERS} ${CANIS_SOURCES})
set_target_properties(${CANIS_ENGINE_LIB} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

target_include_directories(${CANIS_ENGINE_LIB} PUBLIC SDL3::SDL3 stb ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${CANIS_ENGINE_LIB} PUBLIC SDL3::SDL3 stb)

add_executable(${PROJECT_NAME} main.cpp)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLES2 REQUIRED glesv2)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    SDL3::SDL3
    stb
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GLES2_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    SDL3::SDL3
    stb
    ${CANIS_ENGINE_LIB}
    ${GLES2_LIBRARIES}
)

#add_custom_command(TARGET CanisEngine POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:${PROJECT_NAME}>
#        $<TARGET_FILE_DIR:CanisEngine>
#)
