//
// Created by patri on 08.07.2025.
//

#ifndef SWAPCHAINELEMENT_H
#define SWAPCHAINELEMENT_H

#include <vulkan/vulkan.h>

struct SwapchainElement
{
    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkFence fence = VK_NULL_HANDLE;
    VkFence lastFence = VK_NULL_HANDLE;
    VkSemaphore startSemaphore = VK_NULL_HANDLE;
    VkSemaphore endSemaphore = VK_NULL_HANDLE;
};

#endif //SWAPCHAINELEMENT_H
