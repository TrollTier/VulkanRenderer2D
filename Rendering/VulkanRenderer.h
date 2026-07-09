#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <filesystem>

#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <unordered_map>

#include "Buffer.h"
#include "Circle.h"
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
        ImDrawData* uiData);

    uint32_t getPixelsPerUnit() const { return m_pixelsPerUnit; }
    void setPixelsPerUnit(uint32_t pixelsPerUnit) { m_pixelsPerUnit = pixelsPerUnit; }

    const Texture2D& getTexture(size_t index) const
    {
        return *m_textures[index];
    }

    template<typename T>
    ObjectBuffer<T>* registerDataType(const std::string& name, size_t initialSize)
    {
        auto buffer = std::make_unique<ObjectBuffer<T>>(m_vulkanResources, m_imageCount, initialSize);
        auto* rawPtr = buffer.get();
        m_objectBuffers[name] = std::unique_ptr<IGenericBuffer>(std::move(buffer));
        return rawPtr;
    }

private:
    uint32_t m_pixelsPerUnit = 1;
    std::filesystem::path m_assetsBasePath;
    std::vector<VkDescriptorSet> m_defaultDescriptorSets;

    std::shared_ptr<VulkanResources> m_vulkanResources;
    std::vector<std::unique_ptr<Pipeline>> m_pipelines {};

    std::vector<std::unique_ptr<Mesh>> m_meshes;
    std::vector<std::unique_ptr<Texture2D>> m_textures;

    std::vector<std::unique_ptr<Buffer>> m_vertexBuffers{1};
    std::vector<std::unique_ptr<Buffer>> m_indexBuffers{1};
    std::vector<std::unique_ptr<Buffer>> m_cameraBuffers{};

    std::unordered_map<std::string, std::unique_ptr<IGenericBuffer>> m_objectBuffers{};

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
        const IGenericBuffer& objects,
        const VkPipeline& pipeline,
        const SwapchainElement* currentImageElement,
        size_t currentFrameIndex)
    {
        vkCmdBindPipeline(
            currentImageElement->commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipeline);

        std::vector<VkDescriptorSet> descriptorSets{};
        descriptorSets.push_back(m_defaultDescriptorSets[currentFrameIndex]);
        descriptorSets.push_back(objects.getDescriptorSet(currentFrameIndex));

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
        const size_t meshIndex = mesh.getMeshIndex();

        VkBuffer vertexBuffers[] = { m_vertexBuffers[meshIndex]->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(currentImageElement->commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(currentImageElement->commandBuffer, m_indexBuffers[meshIndex]->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

        vkCmdDrawIndexed(
            currentImageElement->commandBuffer,
            mesh.getIndices().size(),
            objects.getCount(),
            0,
            0,
            0);
    }

    static void imageToAttachmentLayout(SwapchainElement* element);
    static void imageToPresentLayout(SwapchainElement* element);
};

#endif //VULKANRENDERER_H
