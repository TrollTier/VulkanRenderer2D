//
// Created by patri on 10.07.2025.
//

#include "VulkanResources.h"

#include <set>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <array>

#include "VulkanWindow.h"

VulkanResources::~VulkanResources()
{
    m_swapchain.reset();

    vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, m_allocator);
    vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayoutObjectsBuffer, m_allocator);
    vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, m_allocator);
    vkDestroyCommandPool(m_logicalDevice, m_commandPool, m_allocator);
    vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
    vkDestroyDevice(m_logicalDevice, m_allocator);
    vkDestroyInstance(m_instance, m_allocator);
}

void VulkanResources::initialize(
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

    const auto windowExtent = m_window->getWindowExtent();
    m_swapchain = std::make_shared<Swapchain>(
        m_physicalDevice,
        m_logicalDevice,
        m_surface,
        m_commandPool,
        m_allocator,
        windowExtent.width,
        windowExtent.height);

    initializeDescriptorPool();
    initializeDescriptorSetLayout();
    initializeObjectsBufferLayout();
}

void VulkanResources::initializeInstance(
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

void VulkanResources::verifyValidationLayerSupport(const std::vector<const char*> &layerNames)
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

VkPhysicalDevice VulkanResources::pickPhysicalDevice()
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

void VulkanResources::initializeLogicalDevice()
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


uint32_t VulkanResources::getQueueFamilyIndex(VkQueueFlags queueFlags)
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

void VulkanResources::initializeDescriptorPool()
{
    const auto swapchainImageCount = m_swapchain->getImageCount();

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_physicalDevice, &properties);

    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = swapchainImageCount;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = swapchainImageCount * 100;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = swapchainImageCount;

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.maxSets = swapchainImageCount * 10000;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    const VkResult result = vkCreateDescriptorPool(
        m_logicalDevice,
        &descriptorPoolInfo,
        m_allocator,
        &m_descriptorPool);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool");
    }
}

void VulkanResources::recreateSwapchain()
{
    const auto windowExtent = m_window->getWindowExtent();
    vkDeviceWaitIdle(m_logicalDevice);

    m_swapchain.reset();
    m_swapchain = std::make_shared<Swapchain>(
        m_physicalDevice,
        m_logicalDevice,
        m_surface,
        m_commandPool,
        m_allocator,
        windowExtent.width,
        windowExtent.height);
}

void VulkanResources::initializeDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding cameraBinding{};
    cameraBinding.binding = 0;
    cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraBinding.descriptorCount = 1;
    cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 1;
    samplerBinding.descriptorCount = 12;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.pImmutableSamplers = nullptr;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { cameraBinding, samplerBinding };
    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();

    const VkResult result = vkCreateDescriptorSetLayout(
        m_logicalDevice,
        &createInfo,
        m_allocator,
        &m_descriptorSetLayout);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}

void VulkanResources::initializeObjectsBufferLayout()
{
    VkDescriptorSetLayoutBinding objectsBufferBinding{};
    objectsBufferBinding.binding = 0;
    objectsBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    objectsBufferBinding.descriptorCount = 1;
    objectsBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = 1;
    createInfo.pBindings = &objectsBufferBinding;

    const VkResult result = vkCreateDescriptorSetLayout(
        m_logicalDevice,
        &createInfo,
        m_allocator,
        &m_descriptorSetLayoutObjectsBuffer);

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}
