//
// Created by patri on 16.07.2025.
//

#ifndef PIPELINE_H
#define PIPELINE_H
#include <memory>
#include <vulkan/vulkan.h>

#include "UniformBufferObject.h"
#include "VulkanRessources.h"

class Pipeline
{
public:
    ~Pipeline();
    Pipeline(
        std::shared_ptr<VulkanRessources> ressources,
        std::string vertexShaderPath,
        std::string fragmentShaderPath,
        size_t swapchainImageCount,
        VkFormat swapchainImageFormat);

    void updateAfterImageLoaded(VkDescriptorImageInfo& imageInfo);
    void updateUniformBuffer(size_t index, const UniformBufferObject& bufferObject);

    [[nodiscard]] VkPipelineLayout getLayout() const
    {
        return m_pipelineLayout;
    }

    [[nodiscard]] VkDescriptorSet getDescriptorSet(size_t index) const
    {
        return m_descriptorSets[index];
    }

    [[nodiscard]] VkDescriptorSetLayout getDescriptorSetLayout() const
    {
        return m_descriptorSetLayout;
    }

    [[nodiscard]] VkDescriptorPool getDescriptorPool() const
    {
        return m_descriptorPool;
    }

    [[nodiscard]] VkPipeline getPipeline() const
    {
        return m_pipeline;
    }

private:
    std::shared_ptr<VulkanRessources> m_vulkanRessources;

    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets{};

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBufferMemories;
    std::vector<void*> m_uniformBuffersMapped;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline;
};

#endif //PIPELINE_H
