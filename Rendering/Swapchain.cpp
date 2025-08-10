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

Swapchain::Swapchain(std::shared_ptr<VulkanResources> ressources)
{
    m_ressources = ressources;

    VkSurfaceCapabilitiesKHR capabilities;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_ressources->m_physicalDevice, m_ressources->m_surface, &capabilities) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to query surface capabilities!");
    }

    const auto windowExtent = ressources->getWindow()->getWindowExtent();

    m_width = std::clamp(windowExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    m_height = std::clamp(windowExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    m_format = findFormat(m_ressources->m_physicalDevice, m_ressources->m_surface);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_ressources->m_surface;
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

    if(vkCreateSwapchainKHR(m_ressources->m_logicalDevice, &createInfo, m_ressources->m_allocator, &m_swapchain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swapchain!");
    }

    uint32_t imageCount;
    if (vkGetSwapchainImagesKHR(m_ressources->m_logicalDevice, m_swapchain, &imageCount, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to query swapchain images!");
    }

    std::vector<VkImage> images(imageCount);
    if(vkGetSwapchainImagesKHR(m_ressources->m_logicalDevice, m_swapchain, &imageCount, images.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to query swapchain images!");
    }

    m_swapChainElements.resize(imageCount);
    std::vector<VkCommandBuffer> commandBuffers(imageCount);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.commandBufferCount = imageCount;
    allocInfo.commandPool = m_ressources->m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    if (vkAllocateCommandBuffers(m_ressources->m_logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    for (size_t i = 0; i < imageCount; i++)
    {
        VkImageView imageView = createImageView(
            m_ressources->m_logicalDevice,
            images[i],
            m_format.format,
            VK_IMAGE_ASPECT_COLOR_BIT,
            m_ressources->m_allocator);

        VkSemaphore startSemaphore = VK_NULL_HANDLE;
        VkSemaphore endSemaphore = VK_NULL_HANDLE;
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(m_ressources->m_logicalDevice, &semaphoreCreateInfo, m_ressources->m_allocator, &startSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create start semaphore!");
        }

        if (vkCreateSemaphore(m_ressources->m_logicalDevice, &semaphoreCreateInfo, m_ressources->m_allocator, &endSemaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create end semaphore!");
        }

        VkFence fence = VK_NULL_HANDLE;
        VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        if (vkCreateFence(m_ressources->m_logicalDevice, &fenceCreateInfo, m_ressources->m_allocator, &fence) != VK_SUCCESS)
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
        vkDestroyImageView(m_ressources->m_logicalDevice, element.imageView, m_ressources->m_allocator);
        vkDestroySemaphore(m_ressources->m_logicalDevice, element.endSemaphore, m_ressources->m_allocator);
        vkDestroySemaphore(m_ressources->m_logicalDevice, element.startSemaphore, m_ressources->m_allocator);
        vkDestroyFence(m_ressources->m_logicalDevice, element.fence, m_ressources->m_allocator);
        vkFreeCommandBuffers(m_ressources->m_logicalDevice, m_ressources->m_commandPool, 1, &element.commandBuffer);
    }

    vkDestroySwapchainKHR(m_ressources->m_logicalDevice, m_swapchain, m_ressources->m_allocator);
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

