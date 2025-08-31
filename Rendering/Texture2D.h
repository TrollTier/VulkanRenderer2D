//
// Created by patri on 17.07.2025.
//

#ifndef TEXTURE2D_H
#define TEXTURE2D_H
#include <string>
#include <vulkan/vulkan.h>
#include <memory>

#include "Buffer.h"
#include "VulkanResources.h"
#include "../Core/TextureAtlasParser.h"
#include <filesystem>

#include "ImageRect.h"

class Texture2D
{
public:
    Texture2D(
        std::weak_ptr<VulkanResources> vulkanResources,
        const std::filesystem::path& assetsBasePath,
        const AtlasEntry& spriteInfo);
    ~Texture2D();

    [[nodiscard]] VkImageView getImageView() const
    {
        return m_textureImageView;
    }

    [[nodiscard]] uint32_t getWidth() const { return m_textureWidth;}
    [[nodiscard]] uint32_t getHeight() const { return m_textureHeight;}
    [[nodiscard]] const ImageRect& getFrame(size_t index) const { return m_frames[index];}

private:
    std::weak_ptr<VulkanResources> m_vulkanResources;
    VkImage m_textureImage = VK_NULL_HANDLE;
    VkImageView m_textureImageView = VK_NULL_HANDLE;
    VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
    uint32_t m_textureWidth = 0;
    uint32_t m_textureHeight = 0;
    std::vector<ImageRect> m_frames{1};

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
        Buffer& buffer,
        VkImage image,
        uint32_t width,
        uint32_t height);
};

#endif //TEXTURE2D_H
