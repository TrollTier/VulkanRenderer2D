//
// Created by patri on 10.07.2025.
//

#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include <vector>
#include <vulkan/vulkan.h>
#include "../include/glfw-3.4/include/GLFW/glfw3.h"

class VulkanWindow {
public :
    VulkanWindow(GLFWwindow *window);
    VkResult createSurface(VkInstance instance, VkSurfaceKHR* surface);
    void fillRequiredInstanceExtensions(std::vector<const char*>& extensions);

private:
    GLFWwindow* m_window;
};



#endif //VULKANWINDOW_H
