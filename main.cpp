#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN

#include "Core/Game.h"

int main()
{
    Game game{};
    game.RunLoop();

    return 0;
}
