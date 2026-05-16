//
// Created by patri on 08.07.2025.
//

#include "Swapchain.h"

#include <memory>
#include <ranges>
#include <stdexcept>
#include <vector>

#include "VulkanResources.h"

VkSurfaceFormatKHR findFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
    uint32_t formatCount;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to query surface formats!");
    }

    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    if(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to query surface formats!");
    }

    for (const VkSurfaceFormatKHR& format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM)
        {
            return format;
        }
    }

    throw std::runtime_error("failed to find suitable format!");
}

VkImageView createImageView(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags aspectFlags,
    const VkAllocationCallbacks* allocator)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, allocator, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    return imageView;
}

Swapchain::Swapchain(
    VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    VkSurfaceKHR surface,
    VkCommandPool commandPool,
    VkAllocationCallbacks* allocator,
    uint32_t windowWidth,
    uint32_t windowHeight)
{
    m_allocator = allocator;
    m_commandPool = commandPool;
    m_logicalDevice = logicalDevice;

    VkSurfaceCapabilitiesKHR capabilities;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to query surface capabilities!");
    }

    m_width = std::clamp(
        windowWidth,
        capabilities.minImageExtent.width,
        capabilities.maxImageExtent.width);
    m_height = std::clamp(
        windowHeight,
        capabilities.minImageExtent.height,
        capabilities.maxImageExtent.height);
    m_format = findFormat(physicalDevice, surface);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = capabilities.minImageCount;
    createInfo.imageFormat = m_format.format;
    createInfo.imageColorSpace = m_format.colorSpace;
    createInfo.imageExtent = { m_width, m_height };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    createInfo.clipped = true;

    VkResult result = vkCreateSwapchainKHR(logicalDevice, &createInfo, allocator, &m_swapchain);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swapchain!");
    }

    uint32_t imageCount;
    result = vkGetSwapchainImagesKHR(logicalDevice, m_swapchain, &imageCount, nullptr);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to query swapchain images!");
    }

    std::vector<VkImage> images(imageCount);
    result = vkGetSwapchainImagesKHR(logicalDevice, m_swapchain, &imageCount, images.data());
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to query swapchain images!");
    }

    m_swapChainElements.resize(imageCount);
    std::vector<VkCommandBuffer> commandBuffers(imageCount);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.commandBufferCount = imageCount;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageView imageView = createImageView(
            logicalDevice,
            images[i],
            m_format.format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            allocator);

        VkSemaphore startSemaphore = VK_NULL_HANDLE;
        VkSemaphore endSemaphore = VK_NULL_HANDLE;
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, allocator, &startSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create start semaphore!");
        }

        if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, allocator, &endSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create end semaphore!");
        }

        VkFence fence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(logicalDevice, &fenceCreateInfo, allocator, &fence) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create fence!");
        }

        SwapchainElement element
        {
            images[i],
            imageView,
            commandBuffers[i],
            fence,
            VK_NULL_HANDLE,
            startSemaphore,
            endSemaphore
        };

        m_swapChainElements[i] = element;
    }
}

Swapchain::~Swapchain()
{
    for (const auto element : m_swapChainElements)
    {
        vkDestroyImageView(m_logicalDevice, element.imageView, m_allocator);
        vkDestroySemaphore(m_logicalDevice, element.endSemaphore, m_allocator);
        vkDestroySemaphore(m_logicalDevice, element.startSemaphore, m_allocator);
        vkDestroyFence(m_logicalDevice, element.fence, m_allocator);
        vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &element.commandBuffer);
    }

    vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, m_allocator);
}

SwapchainElement* Swapchain::getCurrentFrame()
{
    return &m_swapChainElements[m_currentFrame];
}

SwapchainElement* Swapchain::getFrameAt(size_t index)
{
    return &m_swapChainElements[index];
}

void Swapchain::moveToNextFrame()
{
    m_currentFrame = (m_currentFrame + 1) % m_swapChainElements.size();
}

