cmake_minimum_required(VERSION 3.31)
project(FireEmblemClone CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

add_executable(FireEmblemClone main.cpp
        Core/Timestep.h
        Core/BaseTypes.h
        Core/Mesh.cpp
        Core/Mesh.h
        Core/GameObject.cpp
        Core/GameObject.h
        Core/Sprite.h
        Core/World.cpp
        Core/World.h
        Core/Game.cpp
        Core/Game.h
        Core/Tile.cpp
        Core/Tile.h
        Core/Map.cpp
        Core/Map.h)

set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
add_subdirectory(include/glfw-3.4)

add_subdirectory(include/stb)
add_subdirectory(Rendering)

target_include_directories(${PROJECT_NAME} PUBLIC include/glfw-3.4/include)
target_link_libraries(${PROJECT_NAME} PRIVATE -static Vulkan::Vulkan Rendering glfw stb)