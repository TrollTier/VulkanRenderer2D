//
// Created by patri on 10.07.2025.
//

#ifndef VULKANRESSOURCES_H
#define VULKANRESSOURCES_H

#include <memory>
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

    explicit VulkanRessources(std::shared_ptr<VulkanWindow> window): m_window(window) {}
    ~VulkanRessources();

    void initialize(
        bool enableValidationLayers,
        const std::vector<const char*>& validationLayers,
        const std::vector<const char*>& instanceExtensions);

    const std::shared_ptr<VulkanWindow> getWindow() const
    {
        return m_window;
    }

private:
    std::shared_ptr<VulkanWindow> m_window;

    const std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
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
