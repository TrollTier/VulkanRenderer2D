#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include <vector>

#include "Buffer.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "Texture2D.h"
#include "VulkanRessources.h"
#include "../Core/Camera.h"
#include "../Core/Map.h"
#include "../Core/Mesh.h"
#include "../Core/World.h"
#include "../include/imgui/imgui.h"

class VulkanRenderer
{
public:
    VulkanRenderer(std::shared_ptr<VulkanRessources> resources, uint32_t pixelsPerUnit);
    ~VulkanRenderer();

    void initialize();
    size_t loadTexture(const char* texturePath);

    void draw_scene(
        const Camera& camera,
        const Map& map,
        const World& world,
        ImDrawData* uiData);

    [[nodiscard]] const Swapchain& getSwapchain() const
    {
        return *m_swapchain;
    }

private:
    uint32_t m_pixelsPerUnit = 1;
    std::vector<VkDescriptorSet> m_defaultDescriptorSets;

    std::shared_ptr<VulkanRessources> m_vulkanRessources;
    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<Pipeline> m_pipeline;

    std::vector<std::unique_ptr<Mesh>> m_meshes;
    std::vector<std::unique_ptr<Texture2D>> m_textures;

    std::vector<VkBuffer> m_vertexBuffers {1, VK_NULL_HANDLE};
    std::vector<VkDeviceMemory> m_vertexBufferMemories {1, VK_NULL_HANDLE};

    std::vector<VkBuffer> m_indexBuffers {1, VK_NULL_HANDLE};
    std::vector<VkDeviceMemory> m_indexBufferMemories {1, VK_NULL_HANDLE};

    std::vector<std::unique_ptr<Buffer>> m_cameraBuffers{};

    std::vector<std::unique_ptr<Buffer>> m_objectStagingBuffers{};
    std::vector<std::unique_ptr<Buffer>> m_objectBuffers{};
    std::vector<VkDescriptorSet> m_objectBufferDescriptors{};

    VkSampler m_sampler = VK_NULL_HANDLE;

    void initializeSampler();
    void initializeDefaultMeshes();
    void onMeshCreated(const Mesh& mesh);

    void createBufferWithData(
        const void* srcData,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer& dstBuffer,
        VkDeviceMemory& dstBufferMemory
    );

    void updateCamera(const Camera& camera, size_t imageIndex);
    uint32_t updateObjectsBuffer(
        VkCommandBuffer commandBuffer,
        size_t imageIndex,
        const Camera& camera,
        const Map& map,
        const World& world);

    static void imageToAttachmentLayout(SwapchainElement* element);
    static void imageToPresentLayout(SwapchainElement* element);
};

#endif //VULKANRENDERER_H
