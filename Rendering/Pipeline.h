//
// Created by patri on 16.07.2025.
//

#ifndef PIPELINE_H
#define PIPELINE_H
#include <memory>
#include <vulkan/vulkan.h>

#include "InstanceData.h"
#include "VulkanResources.h"

class Pipeline
{
public:
    ~Pipeline();
    Pipeline(
        std::shared_ptr<VulkanResources> ressources,
        std::string vertexShaderPath,
        std::string fragmentShaderPath,
        size_t swapchainImageCount,
        VkFormat swapchainImageFormat);

    [[nodiscard]] VkPipelineLayout getLayout() const
    {
        return m_pipelineLayout;
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

    [[nodiscard]] VkDescriptorSetLayout getDescriptorSetLayoutObjectsBuffer() const
    {
        return m_descriptorSetLayoutObjectsBuffer;
    }

private:
    std::shared_ptr<VulkanResources> m_vulkanRessources;

    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayoutObjectsBuffer = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline;

    void initializeDescriptorSetLayout();
    void initializeObjectsBufferLayout();
    void initializeDescriptorPool(size_t swapchainImageCount);
};

#endif //PIPELINE_H
