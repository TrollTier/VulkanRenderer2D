#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "vulkan/vulkan.h"
#include "../include/glfw-3.4/include/GLFW/glfw3.h"
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "Buffer.h"
#include "InstanceData.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "Texture2D.h"
#include "VulkanRessources.h"
#include "../Core/GameObject.h"
#include "../Core/Map.h"
#include "../Core/Mesh.h"
#include "../Core/World.h"

class VulkanRenderer
{
public:
    ~VulkanRenderer();

    void initialize(bool enableValidationLayers, std::shared_ptr<VulkanWindow> window);
    size_t loadTexture(const char* texturePath);

    void draw_scene(const Map& map, const World& world);

private:
    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

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

    std::vector<InstanceData> m_instances {1};
    std::vector<std::unique_ptr<Buffer>> m_cameraBuffers{};

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

    void updateCamera(size_t imageIndex);
    void updateUniformBuffer(VkCommandBuffer commandBuffer, const GameObject& gameObject, const InstanceData& instance);

    static void imageToAttachmentLayout(SwapchainElement* element);

    static void imageToPresentLayout(SwapchainElement* element);
};

#endif //VULKANRENDERER_H
