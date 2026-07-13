//
// Created by patri on 16.07.2025.
//

#ifndef PIPELINE_H
#define PIPELINE_H

#include <memory>
#include <vulkan/vulkan.h>
#include "VulkanResources.h"
#include "Shader.h"

class Pipeline
{
public:
    ~Pipeline();
    Pipeline(
        std::weak_ptr<VulkanResources> resources,
        const Shader& shader,
        VkFormat swapchainImageFormat,
        size_t dataBufferIndex);

    [[nodiscard]] VkPipeline getPipeline() const
    {
        return m_pipeline;
    }

    [[nodiscard]] size_t getDataBufferIndex() const
    {
        return m_dataBufferIndex;;
    }

private:
    std::weak_ptr<VulkanResources> m_vulkanResources;
    VkPipeline m_pipeline;
    size_t m_dataBufferIndex;
};

#endif //PIPELINE_H
