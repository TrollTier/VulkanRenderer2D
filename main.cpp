#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "include/glfw-3.4/include/GLFW/glfw3.h"

#include <iostream>
#include "Rendering/VulkanRenderer.h"
#include <glm/glm.hpp>

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto window = glfwCreateWindow(800, 600, "Vulkan API Tutorial", nullptr, nullptr);
    glfwSetWindowUserPointer(window, nullptr);
    glfwSetFramebufferSizeCallback(window, nullptr);

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

    auto objects = std::vector<GameObject>{};
    objects.emplace_back(
        0,
        glm::vec3(0.0f, 0.0f, 0),
        quadMesh);

    renderer.onGameObjectCreated(objects[0]);

    objects.emplace_back(
        1,
        glm::vec3(64.0f, 0.0f, 0),
        quadMesh);

    renderer.onGameObjectCreated(objects[1]);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        renderer.draw_scene(objects);
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
