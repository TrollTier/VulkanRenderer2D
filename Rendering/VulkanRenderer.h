#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "vulkan/vulkan.h"
#include "../include/glfw-3.4/include/GLFW/glfw3.h"
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

#include "Pipeline.h"
#include "Swapchain.h"
#include "Texture2D.h"
#include "VulkanRessources.h"
#include "../Core/GameObject.h"
#include "../Core/Mesh.h"

class VulkanRenderer
{
public:
    ~VulkanRenderer();

    void initialize(bool enableValidationLayers, VulkanWindow& window);
    void onMeshCreated(const std::shared_ptr<Mesh> mesh);
    void draw_scene(const std::vector<GameObject>& gameObjects);

private:
    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    std::shared_ptr<VulkanRessources> m_vulkanRessources;
    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<Texture2D> m_texture;

    VkSampler m_sampler = VK_NULL_HANDLE;

    std::vector<VkBuffer> m_vertexBuffers {1, VK_NULL_HANDLE};
    std::vector<VkDeviceMemory> m_vertexBufferMemories {1, VK_NULL_HANDLE};

    std::vector<VkBuffer> m_indexBuffers {1, VK_NULL_HANDLE};
    std::vector<VkDeviceMemory> m_indexBufferMemories {1, VK_NULL_HANDLE};

    void createBufferWithData(
        const void* srcData,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkBuffer& dstBuffer,
        VkDeviceMemory& dstBufferMemory
    );

    void updateUniformBuffer(size_t imageIndex);
    void imageToAttachmentLayout(SwapchainElement* element);
    void imageToPresentLayout(SwapchainElement* element);
};

#endif //VULKANRENDERER_H
