#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include <filesystem>

#include "vulkan/vulkan.h"
#include <GLFW/glfw3.h>
#include <vector>

#include "Buffer.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "Texture2D.h"
#include "VulkanResources.h"
#include "../Core/Camera.h"
#include "../Core/Map.h"
#include "../Core/Mesh.h"
#include "../Core/World.h"
#include "../include/imgui/imgui.h"
#include "../Core/TextureAtlasParser.h"

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

    void draw_scene(
        const Camera& camera,
        const Map& map,
        const World& world,
        const std::vector<AtlasEntry>& atlasEntries,
        ImDrawData* uiData);

    [[nodiscard]] const Swapchain& getSwapchain() const
    {
        const Swapchain& ptr = *m_swapchain;
        return ptr;
    }

private:
    uint32_t m_pixelsPerUnit = 1;
    std::filesystem::path m_assetsBasePath;
    std::vector<VkDescriptorSet> m_defaultDescriptorSets;

    std::shared_ptr<VulkanResources> m_vulkanRessources;
    std::unique_ptr<Swapchain> m_swapchain;
    std::unique_ptr<Pipeline> m_pipeline;

    std::vector<std::unique_ptr<Mesh>> m_meshes;
    std::vector<std::unique_ptr<Texture2D>> m_textures;

    std::vector<std::unique_ptr<Buffer>> m_vertexBuffers{1};
    std::vector<std::unique_ptr<Buffer>> m_indexBuffers{1};
    std::vector<std::unique_ptr<Buffer>> m_cameraBuffers{};

    std::vector<std::unique_ptr<Buffer>> m_objectStagingBuffers{};
    std::vector<std::unique_ptr<Buffer>> m_objectBuffers{};
    std::vector<VkDescriptorSet> m_objectBufferDescriptors{};

    VkSampler m_sampler = VK_NULL_HANDLE;

    void initializeSampler();
    void initializeDefaultMeshes();
    void onMeshCreated(const Mesh& mesh);

    void updateCamera(const Camera& camera, size_t imageIndex);
    uint32_t updateObjectsBuffer(
        VkCommandBuffer commandBuffer,
        size_t imageIndex,
        const Camera& camera,
        const Map& map,
        const World& world,
        const std::vector<AtlasEntry>& atlasEntries);

    static void imageToAttachmentLayout(SwapchainElement* element);
    static void imageToPresentLayout(SwapchainElement* element);
};

#endif //VULKANRENDERER_H
