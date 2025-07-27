//
// Created by patri on 10.07.2025.
//

#include "VulkanRessources.h"

#include <set>
#include <stdexcept>
#include <string.h>
#include <vector>

#include "VulkanWindow.h"

VulkanRessources::~VulkanRessources()
{
    vkDestroyCommandPool(m_logicalDevice, m_commandPool, m_allocator);
    vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
    vkDestroyDevice(m_logicalDevice, m_allocator);
    vkDestroyInstance(m_instance, m_allocator);
}

void VulkanRessources::initialize(
    bool enableValidationLayers,
    const std::vector<const char*>& validationLayers,
    const std::vector<const char*>& instanceExtensions)
{
    initializeInstance(enableValidationLayers, validationLayers, instanceExtensions);

    if (m_window->createSurface(m_instance, &m_surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create Vulkan surface");
    }

    m_physicalDevice = pickPhysicalDevice();
    m_graphicsQueueFamilyIndex = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);

    initializeLogicalDevice();
    vkGetDeviceQueue(m_logicalDevice, m_graphicsQueueFamilyIndex, 0, &m_graphicsQueue);

    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(m_logicalDevice, &commandPoolCreateInfo, m_allocator, &m_commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void VulkanRessources::initializeInstance(
    bool enableValidationLayers,
    const std::vector<const char*>& validationLayers,
    const std::vector<const char*>& instanceExtensions)
{
    VkApplicationInfo appInfo = {};
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

    VkInstanceCreateInfo createInfo = {};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    createInfo.ppEnabledExtensionNames = instanceExtensions.data();

    if (enableValidationLayers)
    {
        verifyValidationLayerSupport(validationLayers);

        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateInstance(&createInfo, m_allocator, &m_instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

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

void VulkanRessources::verifyValidationLayerSupport(const std::vector<const char*> &layerNames)
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

VkPhysicalDevice VulkanRessources::pickPhysicalDevice()
{
    uint32_t physicalDeviceCount = 0;
    if (vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to get physical device count!");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    if (vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physicalDevices.data()) != VK_SUCCESS)
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

        std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
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

void VulkanRessources::initializeLogicalDevice()
{
    float priorities[1] = { 1.0f };
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = priorities ;

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRendering
    {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceDescriptorIndexingFeaturesEXT physicalDeviceDescriptorIndexingFeatures{};
    physicalDeviceDescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT;
    physicalDeviceDescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    physicalDeviceDescriptorIndexingFeatures.runtimeDescriptorArray = VK_TRUE;
    physicalDeviceDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    physicalDeviceDescriptorIndexingFeatures.pNext = &dynamicRendering;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.pNext = &physicalDeviceDescriptorIndexingFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    deviceCreateInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(m_physicalDevice, &deviceCreateInfo, m_allocator, &m_logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create logical device!");
    }
}


uint32_t VulkanRessources::getQueueFamilyIndex(VkQueueFlags queueFlags)
{
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, nullptr);

    if (familyCount == 0)
    {
        throw std::runtime_error("failed to find any queue families!");
    }

    std::vector<VkQueueFamilyProperties> familyProperties(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &familyCount, familyProperties.data());

    for (size_t i = 0; i < familyProperties.size(); i++)
    {
        if (familyProperties[i].queueFlags & queueFlags)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable queue families!");
}