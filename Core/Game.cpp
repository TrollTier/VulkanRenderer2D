//
// Created by patri on 24.07.2025.
//

#include "Game.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <chrono>

#include <iostream>
#include <random>

#include "../Rendering/VulkanRenderer.h"

#include "../Core/MeshLoader.h"
#include "../Core/World.h"

Game::Game()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(1280, 768, "Vulkan API Tutorial", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, nullptr);

    m_vulkanWindow = std::make_shared<VulkanWindow>(m_window);
    m_renderer = std::make_unique<VulkanRenderer>();
    m_renderer->initialize(true, m_vulkanWindow);

    m_meshLoader = std::make_unique<MeshLoader>();
    const auto& quadMesh = m_meshLoader->getQuadMeshShared();
    m_renderer->onMeshCreated(quadMesh);

    m_textureIndices.push_back(m_renderer->loadTexture("../Assets/texture.jpg"));
    m_textureIndices.push_back(m_renderer->loadTexture("../Assets/Flame.png"));
    m_textureIndices.push_back(m_renderer->loadTexture("../Assets/wood_1.png"));

    m_world = std::make_unique<World>();
    m_map = std::make_unique<Map>(50, 50, 64);

    std::random_device random;
    std::mt19937 generator(random());

    std::uniform_real_distribution<float> distributionX(0, 1280);
    std::uniform_real_distribution<float> distributionY(0, 768);
    std::uniform_int_distribution<size_t> distributionTextureIndex(0, m_textureIndices.size() - 1);

    for (size_t i = 0; i < 100; i++)
    {
        const auto randomValueX = distributionX(generator);
        const auto randomValueY = distributionY(generator);
        const auto randomTextureIndex = distributionTextureIndex(generator);

        const auto object = m_world->addGameObject(
            glm::vec3(randomValueX, randomValueY, 0),
            quadMesh,
            Sprite{randomTextureIndex});

        m_renderer->onGameObjectCreated(object);
    }

    glfwSetMouseButtonCallback(m_window, Game::glfwMouseButtonHandler);
}

void Game::RunLoop()
{
    while (!glfwWindowShouldClose(m_window))
    {
        const auto startOfFrame = std::chrono::high_resolution_clock::now();

        glfwPollEvents();

        const auto startOfUpdate = std::chrono::high_resolution_clock::now();

        // TODO: poll mouse events to draw tiles on the map

        // TODO: UI for tile selection. IMGUI?

        const auto endOfUpdate = std::chrono::high_resolution_clock::now();
        const auto startOfRender = std::chrono::high_resolution_clock::now();

        m_renderer->draw_scene(m_world->getGameObjects());

        const auto endOfRender = std::chrono::high_resolution_clock::now();

        const auto endOfFrame = std::chrono::high_resolution_clock::now();
        const auto frameDuration = endOfFrame - startOfFrame;
        const auto updateDuration = endOfUpdate - startOfUpdate;
        const auto renderDuration = endOfRender - startOfRender;

        std::cout << "Frame:" << std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count() << std::endl;
        std::cout << "Update:" << std::chrono::duration_cast<std::chrono::milliseconds>(updateDuration).count() << std::endl;
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

    const uint16_t tileSize = m_map->getTileSize();
    const auto tileRow = static_cast<uint16_t>(std::floor(ypos / tileSize));
    const auto tileColumn = static_cast<uint16_t>(std::floor(xpos / tileSize));

    std::random_device random;
    std::mt19937 generator(random());
    std::uniform_int_distribution<size_t> distributionTextureIndex(0, m_textureIndices.size() - 1);
    const size_t textureIndex = distributionTextureIndex(generator);

    auto& tile = m_map->getTileAt(tileColumn, tileRow);
    tile.sprite.textureIndex =  textureIndex;

    const auto object = m_world->addGameObject(
        glm::vec3(static_cast<float>(tileColumn * tileSize), static_cast<float>(tileRow * tileSize), 0),
        this->m_meshLoader->getQuadMesh(),
        Sprite(textureIndex));

    m_renderer->onGameObjectCreated(object);
}


void Game::glfwMouseButtonHandler(GLFWwindow* window, int button, int action, int mods)
{
    auto game = reinterpret_cast<Game*>(glfwGetWindowUserPointer(window));
    game->mouseButtonCallback(button, action, mods);
}
