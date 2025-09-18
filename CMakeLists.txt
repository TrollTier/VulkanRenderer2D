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
        Core/Editor.cpp
        Core/Editor.h
        Core/Tile.cpp
        Core/Tile.h
        Core/Map.cpp
        Core/Map.h
        Core/Camera.cpp
        Core/Camera.h
        Core/ViewFrustrum.h
        Core/SpriteFrame.h
        Core/TextureAtlasParser.h
        Core/TextureAtlasStructures.h
        Core/TileTypes.h
        Core/Animator.cpp
        Core/Animator.h
        Core/AnimationData.h
        Core/AnimationSystem.cpp
        Core/AnimationSystem.h)

set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
add_subdirectory(include/glfw-3.4)

add_subdirectory(include/stb)
add_subdirectory(Rendering)

# Set where the ImGui files are stored
set(IMGUI_PATH  include/imgui)

# Compile as static library
file(GLOB IMGUI_SOURCES ${IMGUI_PATH}/*.cpp ${IMGUI_PATH}/backends/*.cpp)
add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends)
target_link_libraries(ImGui PRIVATE Vulkan::Vulkan glfw)

target_include_directories(${PROJECT_NAME} PUBLIC ${IMGUI_PATH} ${IMGUI_PATH}/backends)
target_link_libraries(${PROJECT_NAME} PUBLIC -static Vulkan::Vulkan Rendering glfw stb ImGui)