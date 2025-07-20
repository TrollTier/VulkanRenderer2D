#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <chrono>

#include "include/glfw-3.4/include/GLFW/glfw3.h"

#include <iostream>
#include <random>

#include "Rendering/VulkanRenderer.h"
#include <glm/glm.hpp>

std::vector<GameObject> createGameObjects(
    VulkanRenderer& renderer,
    const std::shared_ptr<Mesh>& quadMesh)
{
    std::random_device random;
    std::mt19937 generator(random());

    std::uniform_real_distribution<float> distributionX(0, 1280);
    std::uniform_real_distribution<float> distributionY(0, 768);

    auto objects = std::vector<GameObject>{};
    for (size_t i = 0; i < 100; i++)
    {
        const auto randomValueX = distributionX(generator);
        const auto randomValueY = distributionY(generator);

        objects.emplace_back(
            i,
            glm::vec3(randomValueX, randomValueY, 0),
            quadMesh);

        renderer.onGameObjectCreated(objects[i]);
    }

    return std::move(objects);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto window = glfwCreateWindow(1280, 768, "Vulkan API Tutorial", nullptr, nullptr);
    glfwSetWindowUserPointer(window, nullptr);
    glfwSetFramebufferSizeCallback(window, nullptr);
    {
        VulkanWindow vulkanWindow(window);
        VulkanRenderer renderer{};
        renderer.initialize(true, vulkanWindow);

        const std::vector<Vertex> vertices = {
            {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
            {{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        };

        const std::vector<uint16_t> indices = {
            0, 1, 2, 2, 3, 0
        };

        const auto quadMesh = std::make_shared<Mesh>(0, vertices, indices);
        renderer.onMeshCreated(quadMesh);

        std::vector<GameObject> objects = createGameObjects(renderer, quadMesh);

        std::random_device random;
        std::mt19937 generator(random());
        std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

        while (!glfwWindowShouldClose(window)) {
            const auto startOfFrame = std::chrono::high_resolution_clock::now();

            glfwPollEvents();

            for (auto& object : objects)
            {
                const auto randomValueX = distribution(generator);
                const auto randomValueY = distribution(generator);
                object.moveBy(glm::vec3(randomValueX, randomValueY, 0));
            }

            renderer.draw_scene(objects);

            const auto endOfFrame = std::chrono::high_resolution_clock::now();
            const auto frameDuration = endOfFrame - startOfFrame;
            std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count() << std::endl;
        }
    }
    return 0;
}
