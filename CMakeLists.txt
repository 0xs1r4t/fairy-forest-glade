cmake_minimum_required(VERSION 3.16)

# Add this near the top, before project()
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

project(fairy_forest_glade LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/main.cpp
    src/utils/*.cpp
    src/headers/*.h
)
add_executable(fairy_forest_glade ${SOURCES})

# headers location
target_include_directories(fairy_forest_glade PRIVATE ${CMAKE_SOURCE_DIR}/src/headers)

# Find packages with correct names
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(OpenEXR CONFIG REQUIRED)
find_package(IMath CONFIG REQUIRED)

# Stb include directory
if (DEFINED Stb_INCLUDE_DIR)
    target_include_directories(fairy_forest_glade PRIVATE ${Stb_INCLUDE_DIR})
endif()

# Link libraries with correct target names
target_link_libraries(fairy_forest_glade
    PRIVATE
        glfw
        OpenGL::GL
        GLEW::GLEW
        glm::glm
        assimp::assimp
        Imath::Imath
        OpenEXR::OpenEXR
)

# copy runtime assets (shaders, models) next to the exe
add_custom_command(TARGET fairy_forest_glade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:fairy_forest_glade>/shaders
)
add_custom_command(TARGET fairy_forest_glade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/shaders
        $<TARGET_FILE_DIR:fairy_forest_glade>/shaders
)
add_custom_command(TARGET fairy_forest_glade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/textures
        $<TARGET_FILE_DIR:fairy_forest_glade>/textures
)
add_custom_command(TARGET fairy_forest_glade POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/models
        $<TARGET_FILE_DIR:fairy_forest_glade>/models
)