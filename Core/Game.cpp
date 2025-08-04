//
// Created by patri on 24.07.2025.
//

#include "Game.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <chrono>

#include <iostream>
#include <random>
#include <thread>

#include "Timestep.h"
#include "../Rendering/VulkanRenderer.h"

#include "../Core/World.h"

Game::Game()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(1280, 768, "Vulkan API Tutorial", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, nullptr);
    glfwMaximizeWindow(m_window);

    m_vulkanWindow = std::make_shared<VulkanWindow>(m_window);

    std::vector<const char*> instanceExtensions{};
    m_vulkanWindow->fillRequiredInstanceExtensions(instanceExtensions);
    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    m_vulkanRessources = std::make_shared<VulkanRessources>(m_vulkanWindow);
    m_vulkanRessources->initialize(
        true,
        m_validationLayers,
        instanceExtensions);

    m_renderer = std::make_unique<VulkanRenderer>(m_vulkanRessources, PIXELS_PER_UNIT);
    m_renderer->initialize();

    m_textureIndices.push_back(m_renderer->loadTexture("../Assets/default_texture.jpg"));
    m_textureIndices.push_back(m_renderer->loadTexture("../Assets/texture.jpg"));
    m_textureIndices.push_back(m_renderer->loadTexture("../Assets/Flame.png"));
    m_textureIndices.push_back(m_renderer->loadTexture("../Assets/wood_1.png"));

    m_world = std::make_unique<World>();
    m_map = std::make_unique<Map>(50, 50, 1);

    const auto windowExtent = m_vulkanWindow->getWindowExtent();

    const CameraArea visibleArea
    {
        static_cast<float>(windowExtent.width) / static_cast<float>(PIXELS_PER_UNIT),
        static_cast<float>(windowExtent.height) / static_cast<float>(PIXELS_PER_UNIT),
        1.0f,
        10.0f
    };

    m_camera = std::make_unique<Camera>(
        glm::vec3((float)m_map->getColumns() / 2.0f, (float)m_map->getRows() / 2.0f, 0.0f),
        visibleArea,
        windowExtent.width,
        windowExtent.height);

    glfwSetMouseButtonCallback(m_window, Game::glfwMouseButtonHandler);
}

void Game::RunLoop()
{
    auto startOfLastUpdate = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(m_window))
    {
        const auto startOfFrame = std::chrono::high_resolution_clock::now();

        glfwPollEvents();

        auto startOfCurrentUpdate = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float, std::milli> durationSinceLastUpdate =
            startOfCurrentUpdate - startOfLastUpdate;

        const Timestep step
        {
            durationSinceLastUpdate.count(),
            durationSinceLastUpdate.count() / 1000
        };

        handleKeyInput(step);

        startOfLastUpdate = startOfCurrentUpdate;

        const auto startOfRender = std::chrono::high_resolution_clock::now();

        m_renderer->draw_scene(*m_camera, *m_map, *m_world);

        const auto endOfRender = std::chrono::high_resolution_clock::now();

        const auto endOfFrame = std::chrono::high_resolution_clock::now();
        const auto frameDuration = endOfFrame - startOfFrame;
        const auto renderDuration = endOfRender - startOfRender;

        std::cout << "Frame:" << std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count() << std::endl;
        std::cout << "Render:" << std::chrono::duration_cast<std::chrono::milliseconds>(renderDuration).count() << std::endl;
    }
}

void Game::mouseButtonCallback(int button, int action, int mods)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
    {
        return;
    }

    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);

    const auto& frustum = m_camera->getFrustum();

    const auto widthToWorldX = xpos / PIXELS_PER_UNIT + frustum.x;
    const auto heightToWorldY = ypos / PIXELS_PER_UNIT + frustum.y;

    const auto tileRow = static_cast<uint16_t>(std::floor(heightToWorldY));
    const auto tileColumn = static_cast<uint16_t>(std::floor(widthToWorldX));

    auto& tile = m_map->getTileAt(tileColumn, tileRow);
    tile.sprite.textureIndex =  (tile.sprite.textureIndex + 1) % m_textureIndices.size();
}

void Game::handleKeyInput(const Timestep& timestep)
{
    glm::vec3 cameraMovement(0.0f, 0.0f, 0.0f);

    int state = glfwGetKey(m_window, GLFW_KEY_W);
    if (state == GLFW_PRESS)
    {
        cameraMovement.y = -1;
    }

    state = glfwGetKey(m_window, GLFW_KEY_S);
    if (state == GLFW_PRESS)
    {
        cameraMovement.y = 1;
    }

    state = glfwGetKey(m_window, GLFW_KEY_A);
    if (state == GLFW_PRESS)
    {
        cameraMovement.x = -1;
    }

    state = glfwGetKey(m_window, GLFW_KEY_D);
    if (state == GLFW_PRESS)
    {
        cameraMovement.x = 1;
    }

    if (glm::length(cameraMovement) < 0.1f)
    {
        return;
    }

    cameraMovement = glm::normalize(cameraMovement);
    cameraMovement *= 2 * timestep.deltaSeconds;

    m_camera->moveBy(cameraMovement);
}


void Game::glfwMouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{
    auto game = reinterpret_cast<Game*>(glfwGetWindowUserPointer(window));
    game->mouseButtonCallback(button, action, mods);
}
