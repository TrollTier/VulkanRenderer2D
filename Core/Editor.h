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

class Editor
{
public:
    const float ZOOM_STEP_FACTOR = 1.1f;
    int32_t PIXELS_PER_UNIT = 64;

    Editor();

    void RunLoop();
    void mouseButtonCallback(int button, int action, int mods);
    void scrollCallback(double xoffset, double yoffset);

private:
    std::vector<AtlasEntry> m_atlasEntries;

    GLFWwindow* m_window;
    std::shared_ptr<VulkanWindow> m_vulkanWindow;
    std::shared_ptr<VulkanResources> m_vulkanResources;
    std::unique_ptr<VulkanRenderer> m_renderer;
    std::unique_ptr<World> m_world;
    std::unique_ptr<Map> m_map;
    std::vector<size_t> m_textureIndices;
    std::unique_ptr<Camera> m_camera;

    VkDescriptorPool m_imGuiPool = VK_NULL_HANDLE;

    int32_t m_selectedTileType = -1;
    bool m_runAnimations = true;
    bool m_showImGui = true;

    void initImGui();
    void updateAnimations(const Timestep& step);
    void updateUI();
    void setSelectedTile();

    [[nodiscard]] glm::vec2 screenToWorld(const glm::vec2& screenPos) const;

    void handleKeyInput(const Timestep& timestep);
    static void glfwMouseButtonHandler(GLFWwindow* window, int button, int action, int mods);
    static void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void handleWindowResize();
    static void glfwWindowResize(GLFWwindow* window, int width, int height);
};

#endif //GAME_H
