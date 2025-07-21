#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <chrono>

#include "include/glfw-3.4/include/GLFW/glfw3.h"

#include <iostream>
#include <random>

#include "Rendering/VulkanRenderer.h"
#include <glm/glm.hpp>

#include "Core/MeshLoader.h"

std::vector<GameObject> createGameObjects(
    VulkanRenderer& renderer,
    const std::shared_ptr<Mesh>& quadMesh,
    const std::vector<size_t>& textureIndices)
{
    std::random_device random;
    std::mt19937 generator(random());

    std::uniform_real_distribution<float> distributionX(0, 1280);
    std::uniform_real_distribution<float> distributionY(0, 768);
    std::uniform_int_distribution<size_t> distributionTextureIndex(0, textureIndices.size() - 1);

    auto objects = std::vector<GameObject>{};
    for (size_t i = 0; i < 100; i++)
    {
        const auto randomValueX = distributionX(generator);
        const auto randomValueY = distributionY(generator);
        const auto randomTextureIndex = distributionTextureIndex(generator);

        objects.emplace_back(
            i,
            glm::vec3(randomValueX, randomValueY, 0),
            quadMesh,
            Sprite{randomTextureIndex});

        renderer.onGameObjectCreated(objects[i]);
    }

    return std::move(objects);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto window = glfwCreateWindow(1280, 768, "Vulkan API Tutorial", nullptr, nullptr);
    glfwSetWindowUserPointer(window, nullptr);
    glfwSetFramebufferSizeCallback(window, nullptr);

    VulkanWindow vulkanWindow(window);
    VulkanRenderer renderer{};
    renderer.initialize(true, vulkanWindow);

    MeshLoader meshLoader{};
    const auto& quadMesh = meshLoader.getQuadMeshShared();
    renderer.onMeshCreated(quadMesh);

    std::vector<size_t> textureIndices;
    textureIndices.push_back(renderer.loadTexture("../Assets/texture.jpg"));
    textureIndices.push_back(renderer.loadTexture("../Assets/Flame.png"));
    textureIndices.push_back(renderer.loadTexture("../Assets/wood_1.png"));

    std::vector<GameObject> objects = createGameObjects(renderer, quadMesh, textureIndices);

    std::random_device random;
    std::mt19937 generator(random());
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        const auto startOfFrame = std::chrono::high_resolution_clock::now();

        glfwPollEvents();

        const auto startOfUpdate = std::chrono::high_resolution_clock::now();

        for (auto& object : objects)
        {
            const auto randomValueX = distribution(generator);
            const auto randomValueY = distribution(generator);
            object.moveBy(glm::vec3(randomValueX, randomValueY, 0));
        }

        const auto endOfUpdate = std::chrono::high_resolution_clock::now();
        const auto startOfRender = std::chrono::high_resolution_clock::now();

        renderer.draw_scene(objects);

        const auto endOfRender = std::chrono::high_resolution_clock::now();

        const auto endOfFrame = std::chrono::high_resolution_clock::now();
        const auto frameDuration = endOfFrame - startOfFrame;
        const auto updateDuration = endOfUpdate - startOfUpdate;
        const auto renderDuration = endOfRender - startOfRender;

        std::cout << "Frame:" << std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count() << std::endl;
        std::cout << "Update:" << std::chrono::duration_cast<std::chrono::milliseconds>(updateDuration).count() << std::endl;
        std::cout << "Render:" << std::chrono::duration_cast<std::chrono::milliseconds>(renderDuration).count() << std::endl;
    }

    return 0;
}
