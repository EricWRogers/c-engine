cmake_minimum_required(VERSION 3.18)
project(c-engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/dist/${CMAKE_SYSTEM_NAME}>)
set(ASSETS_DIR_NAME assets)

#add_subdirectory(external/GL)
add_subdirectory(external/glm)
#add_subdirectory(external/SDL)
add_subdirectory(external/stb)

# Set where the ImGui files are stored
#set(IMGUI_PATH  "external/imgui")
    
# Compile as static library 
#file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp ${IMGUI_PATH}/misc/cpp/imgui_stdlib.cpp) 
#file(GLOB IMGUI_HEADERS ${IMGUI_PATH}/*.h ${IMGUI_PATH}/backends/imgui_impl_opengl3.h ${IMGUI_PATH}/backends/imgui_impl_sdl2.h ${IMGUI_PATH}/misc/cpp/imgui_stdlib.h) 
#add_library("imgui" STATIC ${IMGUI_SOURCES})
#target_include_directories("imgui" PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends/ ${IMGUI_PATH}/misc/cpp/)
#target_link_libraries("imgui" PRIVATE SDL2main SDL2-static)

file(GLOB_RECURSE SRC_SOURCES src/*.c*)
file(GLOB_RECURSE SRC_HEADERS src/*.h*)

add_executable(${PROJECT_NAME} ${SRC_SOURCES} ${SRC_HEADERS})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        glm
        stb
        #libglew_static
        #SDL3::SDL3
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        glm
        stb
        #SDL3::SDL3
)

# This command will copy your assets folder to your running directory, in order to have access to your shaders, textures, etc
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
