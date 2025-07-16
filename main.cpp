#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include "include/glfw-3.4/include/GLFW/glfw3.h"

#include <iostream>
#include "Rendering/VulkanRenderer.h"

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    const auto window = glfwCreateWindow(800, 600, "Vulkan API Tutorial", nullptr, nullptr);
    glfwSetWindowUserPointer(window, nullptr);
    glfwSetFramebufferSizeCallback(window, nullptr);

    VulkanWindow vulkanWindow(window);
    VulkanRenderer renderer{};
    renderer.initialize(true, vulkanWindow);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        renderer.draw_scene();
    }

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
