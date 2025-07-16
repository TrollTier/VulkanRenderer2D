//
// Created by patri on 22.06.2025.
//

#ifndef VULKANHELPERS_H
#define VULKANHELPERS_H

#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <set>

inline bool hasLayer(const char* const layerName, const std::vector<VkLayerProperties>& availableLayers)
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

inline void verifyValidationLayerSupport(const std::vector<const char*> &layerNames)
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

inline VkPhysicalDevice getPhysicalDevice(VkInstance instance, const std::vector<const char*> &deviceExtensions)
{
    uint32_t physicalDeviceCount = 0;
    if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to get physical device count!");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    if (vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to get physical devices!");
    }

    if (physicalDevices.empty())
    {
        throw std::runtime_error("No physical devices found!");
    }

    VkPhysicalDeviceProperties physicalDeviceProperties{};
    for (const auto& physicalDevice : physicalDevices)
    {
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        if (physicalDeviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            continue;
        }

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

        if (extensionCount == 0)
        {
            continue;
        }

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        if (requiredExtensions.empty())
        {
            return physicalDevice;
        }
    }

    return physicalDevices[0];
}

inline uint32_t getQueueFamilyIndex(VkPhysicalDevice physicalDevice, VkQueueFlags queueFlags)
{
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, nullptr);

    if (familyCount == 0)
    {
        throw std::runtime_error("failed to find any queue families!");
    }

    std::vector<VkQueueFamilyProperties> familyProperties(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &familyCount, familyProperties.data());

    for (size_t i = 0; i < familyProperties.size(); i++)
    {
        if (familyProperties[i].queueFlags & queueFlags)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable queue families!");
}

#endif //VULKANHELPERS_H
