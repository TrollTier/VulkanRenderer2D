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
        std::shared_ptr<VulkanResources> resources,
        std::string vertexShaderPath,
        std::string fragmentShaderPath,
        VkFormat swapchainImageFormat);

    [[nodiscard]] VkPipelineLayout getLayout() const
    {
        return m_pipelineLayout;
    }

    [[nodiscard]] VkPipeline getPipeline() const
    {
        return m_pipeline;
    }

private:
    std::shared_ptr<VulkanResources> m_vulkanResources;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline;
};

#endif //PIPELINE_H
