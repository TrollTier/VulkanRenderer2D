#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <chrono>

#include "include/glfw-3.4/include/GLFW/glfw3.h"

#include <iostream>
#include <random>

#include "Core/Game.h"
#include "Rendering/VulkanRenderer.h"

#include "Core/MeshLoader.h"
#include "Core/World.h"

int main()
{
    Game game{};
    game.RunLoop();

    return 0;
}
