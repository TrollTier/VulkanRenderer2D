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
        std::shared_ptr<VulkanResources> resources,
        const Shader& shader,
        VkFormat swapchainImageFormat);

    [[nodiscard]] VkPipeline getPipeline() const
    {
        return m_pipeline;
    }

private:
    std::shared_ptr<VulkanResources> m_vulkanResources;
    VkPipeline m_pipeline;
};

#endif //PIPELINE_H
