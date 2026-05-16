//
// Created by patri on 08.07.2025.
//

#ifndef SWAPCHAIN_H
#define SWAPCHAIN_H
#include <algorithm>
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>
#include "SwapchainElement.h"

class VulkanResources;

class Swapchain
{
public:
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkSurfaceFormatKHR m_format;
    uint32_t m_width = 0;
    uint32_t m_height = 0;

    Swapchain(
    VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkSurfaceKHR surface,
    VkCommandPool commandPool,
    VkAllocationCallbacks* allocator,
    uint32_t windowWidth,
    uint32_t windowHeight);
    ~Swapchain();

    SwapchainElement* getCurrentFrame();
    SwapchainElement* getFrameAt(size_t index);
    void moveToNextFrame();

    [[nodiscard]] size_t getImageCount() const
    {
        return m_swapChainElements.size();
    }

    [[nodiscard]] size_t getCurrentFrameIndex() const
    {
        return m_currentFrame;;
    }

private:
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkAllocationCallbacks* m_allocator = nullptr;
    std::vector<SwapchainElement> m_swapChainElements;
    size_t m_currentFrame = 0;
};

#endif //SWAPCHAIN_H
