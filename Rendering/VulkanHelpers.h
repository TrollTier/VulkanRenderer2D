//
// Created by patri on 22.06.2025.
//

#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdexcept>
#include <cstring>

class VulkanHelpers
{
public:
    static bool hasLayer(const char* const layerName, const std::vector<VkLayerProperties>& availableLayers)
    {
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                return true;
            }
        }

        return false;
    }

    static void verifyValidationLayerSupport(const std::vector<const char*> &layerNames)
    {
        uint32_t propertyCount = 0;
        if (vkEnumerateInstanceLayerProperties(&propertyCount, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to fetch layer properties count");
        }

        if (propertyCount == 0)
        {
            throw std::runtime_error("failed to find any layer properties");
        }

        std::vector<VkLayerProperties> availableLayers(propertyCount);
        if (vkEnumerateInstanceLayerProperties(&propertyCount, availableLayers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to fetch layer properties");
        }

        for (const char* layerName : layerNames)
        {
            if (!hasLayer(layerName, availableLayers))
            {
                const std::string layerString (layerName);
                throw std::runtime_error("layer " + layerString + "is not supported on this platform");
            }
        }
    }

    static uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            auto maskedProperties = (memoryProperties.memoryTypes[i].propertyFlags & properties);

            if (typeFilter & (1 << i) && maskedProperties == properties) {
                return i;
            }
        }

        throw std::runtime_error("Failed to find suitable memory type");
    }
};


#endif //VULKANHELPERS_H
