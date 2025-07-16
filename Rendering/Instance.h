//
// Created by patri on 08.07.2025.
//

#ifndef INSTANCE_H
#define INSTANCE_H

void initializeInstance(
    VkInstance &instance,
    bool enableValidationLayers,
    const std::vector<const char*> &validationLayers,
    const std::vector<const char*> &instanceExtensions,
    VkAllocationCallbacks* allocator)
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
        VulkanHelpers::verifyValidationLayerSupport(validationLayers);

        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }

    if (vkCreateInstance(&createInfo, allocator, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

#endif //INSTANCE_H
