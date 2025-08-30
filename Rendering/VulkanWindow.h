//
// Created by patri on 10.07.2025.
//

#ifndef VULKANWINDOW_H
#define VULKANWINDOW_H

#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

typedef struct
{
    uint32_t width;
    uint32_t height;
} WindowExtent;

class VulkanWindow {
public :
    explicit VulkanWindow(GLFWwindow *window);
    VkResult createSurface(VkInstance instance, VkSurfaceKHR* surface);
    void fillRequiredInstanceExtensions(std::vector<const char*>& extensions);

    [[nodiscard]] WindowExtent getWindowExtent() const;
private:
    GLFWwindow* m_window;
};



#endif //VULKANWINDOW_H
