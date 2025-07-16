//
// Created by patri on 10.07.2025.
//

#ifndef VULKANRESSOURCES_H
#define VULKANRESSOURCES_H

#include <vector>
#include <vulkan/vulkan.h>

#include "VulkanWindow.h"

class VulkanRessources {
public:
    VkAllocationCallbacks* m_allocator = nullptr;

    VkInstance m_instance = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_logicalDevice = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;

    uint32_t m_graphicsQueueFamilyIndex = 0;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;

    ~VulkanRessources();
    void initialize(
        bool enableValidationLayers,
        const std::vector<const char*>& validationLayers,
        const std::vector<const char*>& instanceExtensions,
        VulkanWindow& window);

private:
    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    void initializeInstance(
        bool enableValidationLayers,
        const std::vector<const char*>& validationLayers,
        const std::vector<const char*>& instanceExtensions);

    VkPhysicalDevice pickPhysicalDevice();
    uint32_t getQueueFamilyIndex(VkQueueFlags flags);
    void initializeLogicalDevice();

    void verifyValidationLayerSupport(const std::vector<const char*>& layerNames);
};

#endif //VULKANRESSOURCES_H
