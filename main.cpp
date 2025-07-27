#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include <iostream>

#include "Core/Game.h"

int main()
{
    Game game{};

    try
    {
        game.RunLoop();
    } catch (std::runtime_error ex)
    {
        std::cout << ex.what() << std::endl;
    }

    return 0;
}
