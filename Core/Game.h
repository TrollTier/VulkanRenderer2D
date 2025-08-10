//
// Created by patri on 24.07.2025.
//

#ifndef GAME_H
#define GAME_H

#include "Map.h"
#include "World.h"
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Timestep.h"
#include "../Rendering/VulkanRenderer.h"

class Game
{
public:
    const uint32_t PIXELS_PER_UNIT = 64;

    Game();

    void RunLoop();
    void mouseButtonCallback(int button, int action, int mods);

private:
    GLFWwindow* m_window;
    std::shared_ptr<VulkanWindow> m_vulkanWindow;
    std::shared_ptr<VulkanRessources> m_vulkanRessources;
    std::unique_ptr<VulkanRenderer> m_renderer;
    std::unique_ptr<World> m_world;
    std::unique_ptr<Map> m_map;
    std::vector<size_t> m_textureIndices;
    std::unique_ptr<Camera> m_camera;

    VkDescriptorPool m_imGuiPool = VK_NULL_HANDLE;

    void initImGui();

    void handleKeyInput(const Timestep& timestep);
    static void glfwMouseButtonHandler(GLFWwindow* window, int button, int action, int mods);

    void handleWindowResize();
    static void glfwWindowResize(GLFWwindow* window, int width, int height);
};

#endif //GAME_H
