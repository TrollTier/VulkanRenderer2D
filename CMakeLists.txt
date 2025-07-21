cmake_minimum_required(VERSION 3.31)
project(FireEmblemClone CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

add_executable(FireEmblemClone main.cpp
        Level.cpp
        Level.h
        Core/Timestep.h
        Core/BaseTypes.h
        include/stb/stb_image.cpp
        Core/Mesh.cpp
        Core/Mesh.h
        Core/GameObject.cpp
        Core/GameObject.h
        Core/Sprite.h)

add_subdirectory(include/glfw-3.4)
add_subdirectory(include/stb)
add_subdirectory(Rendering)

target_link_libraries(${PROJECT_NAME} PRIVATE -static Vulkan::Vulkan Rendering glfw stb)