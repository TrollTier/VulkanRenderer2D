//
// Created by patri on 17.07.2025.
//

#ifndef TEXTURE2D_H
#define TEXTURE2D_H
#include <string>
#include <vulkan/vulkan.h>
#include <memory>

#include "VulkanRessources.h"

class Texture2D
{
public:
    Texture2D(std::shared_ptr<VulkanRessources> vulkanRessources, const char* imagePath);
    ~Texture2D();

    [[nodiscard]] VkImageView getImageView() const
    {
        return m_textureImageView;
    }

private:
    std::shared_ptr<VulkanRessources> m_vulkanRessources;
    VkImage m_textureImage = VK_NULL_HANDLE;
    VkImageView m_textureImageView = VK_NULL_HANDLE;
    VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;

    void createImage(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory);

    void transitionImageLayout(
        VkImage image,
        VkFormat format,
        VkImageLayout oldLayout,
        VkImageLayout newLayout);

    void copyBufferToImage(
        VkBuffer buffer,
        VkImage image,
        uint32_t width,
        uint32_t height);
};

#endif //TEXTURE2D_H
