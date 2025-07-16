#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "vulkan/vulkan.h"
#include "../include/glfw-3.4/include/GLFW/glfw3.h"
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Swapchain.h"
#include "VulkanRessources.h"

class VulkanRenderer
{
public:
    void initialize(bool enableValidationLayers, VulkanWindow& window);
    void draw_scene();
    ~VulkanRenderer();

private:
    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    std::shared_ptr<VulkanRessources> m_vulkanRessources = nullptr;
    std::unique_ptr<Swapchain> m_swapchain = nullptr;

    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets{};

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBufferMemories;
    std::vector<void*> m_uniformBuffersMapped;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;

    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

    VkBuffer m_indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

    VkImage m_textureImage = VK_NULL_HANDLE;
    VkImageView m_textureImageView = VK_NULL_HANDLE;
    VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;

    void createBufferWithData(
        const void* srcData,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer& dstBuffer,
        VkDeviceMemory& dstBufferMemory
    );

    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

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

    void updateUniformBuffer(size_t imageIndex);
    void imageToAttachmentLayout(SwapchainElement* element);
    void imageToPresentLayout(SwapchainElement* element);
};

#endif //VULKANRENDERER_H
