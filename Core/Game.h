//
// Created by patri on 24.07.2025.
//

#ifndef GAME_H
#define GAME_H

#include "Map.h"
#include "World.h"
#include "../include/glfw-3.4/include/GLFW/glfw3.h"
#include "../Rendering/VulkanRenderer.h"

class Game
{
public:
    Game();

    void RunLoop();
    void mouseButtonCallback(int button, int action, int mods);

private:
    GLFWwindow* m_window;
    std::shared_ptr<VulkanWindow> m_vulkanWindow;
    std::unique_ptr<VulkanRenderer> m_renderer;
    std::unique_ptr<World> m_world;
    std::unique_ptr<Map> m_map;
    std::vector<size_t> m_textureIndices;

    static void glfwMouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
};

#endif //GAME_H
