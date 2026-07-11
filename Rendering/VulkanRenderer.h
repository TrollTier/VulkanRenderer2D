#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <filesystem>

#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <unordered_map>

#include "Buffer.h"
#include "Circle.h"
#include "DrawRequest.h"
#include "ObjectBuffer.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "Texture2D.h"
#include "VulkanResources.h"
#include "../Core/Camera.h"
#include "../Core/Mesh.h"
#include "../Core/World.h"
#include "../include/imgui/imgui.h"
#include "../Core/TextureAtlasParser.h"

struct SpriteRenderData;

class VulkanRenderer
{
public:
    VulkanRenderer(
        std::filesystem::path assetsBasePath,
        std::shared_ptr<VulkanResources> resources,
        uint32_t pixelsPerUnit);
    ~VulkanRenderer();

    void initialize();
    size_t loadTexture(const AtlasEntry& spriteInfo);

    void drawScene(
        const Camera& camera,
        const std::vector<DrawRequest>& drawRequests,
        ImDrawData* uiData);

    [[nodiscard]] uint32_t getPixelsPerUnit() const { return m_pixelsPerUnit; }
    void setPixelsPerUnit(uint32_t pixelsPerUnit) { m_pixelsPerUnit = pixelsPerUnit; }

    [[nodiscard]] const Texture2D& getTexture(size_t index) const
    {
        return *m_textures[index];
    }

    template<typename T>
    size_t registerDataType(size_t initialSize)
    {
        m_objectBuffers.emplace_back(
            std::make_unique<ObjectBuffer<T>>(
                m_vulkanResources,
                m_imageCount,
                initialSize));
        return m_objectBuffers.size() - 1;
    }

    template<typename T>
    ObjectBuffer<T>& getDataBuffer(size_t index)
    {
        IGenericBuffer& reference = *m_objectBuffers[index];
        return static_cast<ObjectBuffer<T>&>(reference);
    }

    template<typename T>
    const ObjectBuffer<T>& getDataBuffer(size_t index) const
    {
        const IGenericBuffer& reference = *m_objectBuffers[index];
        return static_cast<ObjectBuffer<T>&>(reference);
    }

private:
    uint32_t m_pixelsPerUnit = 1;
    std::filesystem::path m_assetsBasePath;
    std::vector<VkDescriptorSet> m_sceneDataDescriptorSets;
    std::vector<VkDescriptorSet> m_frameDataDescriptorSets;

    std::shared_ptr<VulkanResources> m_vulkanResources;
    std::vector<std::unique_ptr<Pipeline>> m_pipelines {};

    std::vector<std::unique_ptr<Mesh>> m_meshes;
    std::vector<std::unique_ptr<Texture2D>> m_textures;

    std::vector<std::unique_ptr<Buffer>> m_vertexBuffers{1};
    std::vector<std::unique_ptr<Buffer>> m_indexBuffers{1};
    std::vector<std::unique_ptr<Buffer>> m_cameraBuffers{};
    std::vector<std::unique_ptr<Buffer>> m_instanceIndexBuffers{};

    std::vector<std::unique_ptr<IGenericBuffer>> m_objectBuffers{};
    std::vector<DrawRequest> m_drawRequests{};
    std::vector<uint32_t> m_instanceIndices{10000};

    VkSampler m_sampler = VK_NULL_HANDLE;
    size_t m_currentDrawIndex = 0;
    size_t m_imageCount = 0;

    void initializeSampler();
    void initializeDefaultMeshes();
    void onMeshCreated(const Mesh& mesh);

    void updateCamera(const Camera& camera, size_t imageIndex);
    void updateObjectBuffers(
        VkCommandBuffer commandBuffer,
        size_t imageIndex);

    void drawIndexed(
        const SwapchainElement* currentImageElement,
        size_t currentFrameIndex,
        size_t pipelineIndex,
        size_t bufferIndex,
        size_t firstDataInstanceIndex,
        size_t lastDataInstanceIndex)
    {
        VkPipeline pipeline = m_pipelines[pipelineIndex]->getPipeline();
        const IGenericBuffer& objects = *m_objectBuffers[bufferIndex];

        vkCmdBindPipeline(
            currentImageElement->commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline);

        std::vector<VkDescriptorSet> descriptorSets{};
        descriptorSets.push_back(m_sceneDataDescriptorSets[currentFrameIndex]);
        descriptorSets.push_back(objects.getDescriptorSet(currentFrameIndex));
        descriptorSets.push_back(m_frameDataDescriptorSets[currentFrameIndex]);

        // Bind global descriptor set
        vkCmdBindDescriptorSets(
            currentImageElement->commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_vulkanResources->m_pipelineLayout,
            0,
            descriptorSets.size(),
            descriptorSets.data(),
            0,
            nullptr
        );

        const Mesh& mesh = *m_meshes[0];

        vkCmdDrawIndexed(
            currentImageElement->commandBuffer,
            mesh.getIndices().size(),
            (lastDataInstanceIndex - firstDataInstanceIndex) + 1,
            0,
            0,
            firstDataInstanceIndex);
    }

    static void imageToAttachmentLayout(SwapchainElement* element);
    static void imageToPresentLayout(SwapchainElement* element);
};

#endif //VULKANRENDERER_H
