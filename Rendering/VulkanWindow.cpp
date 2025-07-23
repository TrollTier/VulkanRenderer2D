//
// Created by patri on 10.07.2025.
//

#include "VulkanWindow.h"

#include <stdexcept>

VulkanWindow::VulkanWindow(GLFWwindow *window)
{
    m_window = window;
}

VkResult VulkanWindow::createSurface(VkInstance instance, VkSurfaceKHR *surface)
{
    const auto result = glfwCreateWindowSurface(instance, m_window, nullptr, surface);
    return result;
}

void VulkanWindow::fillRequiredInstanceExtensions(std::vector<const char *>& extensions)
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    for (const auto extension : requiredExtensions)
    {
        extensions.push_back(extension);
    }
}

WindowExtent VulkanWindow::getWindowExtent() const
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);

    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window, &width, &height);
        glfwWaitEvents();
    }

    return WindowExtent(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
}



