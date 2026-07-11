#include "VulkanRenderer.h"
#include <stdexcept>
#include "VulkanHelpers.h"
#include <fstream>
#include "../Core/Vertex.h"
#include "Swapchain.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

#include "CameraUniformData.h"
#include "VulkanResources.h"
#include "VulkanWindow.h"
#include "GLFW/glfw3native.h"

#include "../Core/Camera.h"

#include "../include/imgui/imgui.h"
#include "../include/imgui/backends/imgui_impl_vulkan.h"

VulkanRenderer::VulkanRenderer(
    std::filesystem::path assetsBasePath,
    std::shared_ptr<VulkanResources> resources,
    uint32_t pixelsPerUnit)
{
    m_assetsBasePath = assetsBasePath;
    m_vulkanResources = resources;
    m_pixelsPerUnit = pixelsPerUnit;
}

VulkanRenderer::~VulkanRenderer()
{
    const auto device = m_vulkanResources->m_logicalDevice;
    const auto allocator = m_vulkanResources->m_allocator;
    const auto& descriptorPool =  m_vulkanResources->m_descriptorPool;

    vkDeviceWaitIdle(device);

    m_cameraBuffers.clear();
    m_textures.clear();
    m_vertexBuffers.clear();
    m_indexBuffers.clear();
    m_pipelines.clear();

    vkFreeDescriptorSets(
        device,
        descriptorPool,
        m_sceneDataDescriptorSets.size(),
        m_sceneDataDescriptorSets.data());
    m_sceneDataDescriptorSets.clear();

    vkDestroySampler(device, m_sampler, allocator);
}

void VulkanRenderer::initialize()
{
    const auto swapchain = m_vulkanResources->getSwapchain().lock();
    m_imageCount = swapchain->getImageCount();

    const Shader spriteShader(
        m_vulkanResources->m_logicalDevice,
        "../Assets/Shaders/vert.spv",
        "../Assets/Shaders/frag.spv");

    m_pipelines.emplace_back(
        std::make_unique<Pipeline>(
            m_vulkanResources,
            spriteShader,
            swapchain->m_format.format));

    const Shader circleShader(
        m_vulkanResources->m_logicalDevice,
        "../Assets/Shaders/Circle/circle_vert.spv",
        "../Assets/Shaders/Circle/circle_frag.spv");

    m_pipelines.emplace_back(
        std::make_unique<Pipeline>(
            m_vulkanResources,
            circleShader,
            swapchain->m_format.format));

    const Shader rectShader(
        m_vulkanResources->m_logicalDevice,
        "../Assets/Shaders/Rectangles/rectangle_vert.spv",
        "../Assets/Shaders/Rectangles/rectangle_frag.spv");

    m_pipelines.emplace_back(
        std::make_unique<Pipeline>(
            m_vulkanResources,
            rectShader,
            swapchain->m_format.format));

    initializeSampler();
    initializeDefaultMeshes();

    const auto imageCount = swapchain->getImageCount();
    m_cameraBuffers.reserve(imageCount);
    m_sceneDataDescriptorSets.resize(imageCount);
    m_frameDataDescriptorSets.resize(imageCount);

    std::vector<VkDescriptorSetLayout> layouts(imageCount, m_vulkanResources->m_descriptorSetLayoutFrameGlobals);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vulkanResources->m_descriptorPool;
    allocInfo.descriptorSetCount = imageCount;
    allocInfo.pSetLayouts = layouts.data();

    const auto result = vkAllocateDescriptorSets(
        m_vulkanResources->m_logicalDevice,
        &allocInfo,
        m_frameDataDescriptorSets.data());

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (int i = 0; i < imageCount; i++)
    {
        m_cameraBuffers.emplace_back(
            std::make_unique<Buffer>(
                m_vulkanResources,
                sizeof(CameraUniformData),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        m_instanceIndexBuffers.emplace_back(
            std::make_unique<Buffer>(
                m_vulkanResources,
                sizeof(uint32_t) * 10000, // TODO: Make this more dynamic and potentially resizable
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        const auto set = m_frameDataDescriptorSets[i];
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_instanceIndexBuffers[i]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = m_instanceIndexBuffers[i]->getSize();

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = set;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(
            m_vulkanResources->m_logicalDevice,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr);
    }
}

void VulkanRenderer::initializeDefaultMeshes()
{
    const std::vector<Vertex> vertices = {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    m_meshes.emplace_back(std::make_unique<Mesh>(0, vertices, indices));
    onMeshCreated(*m_meshes[m_meshes.size() - 1]);
}

size_t VulkanRenderer::loadTexture(const AtlasEntry& spriteInfo)
{
    m_textures.emplace_back(std::make_unique<Texture2D>(m_vulkanResources, m_assetsBasePath, spriteInfo));

    const auto swapchain = m_vulkanResources->getSwapchain().lock();
    const size_t imageCount = swapchain->getImageCount();

    for (size_t i = 0; i < imageCount; i++)
    {
        if (m_sceneDataDescriptorSets[i] != VK_NULL_HANDLE)
        {
            vkFreeDescriptorSets(
                m_vulkanResources->m_logicalDevice,
                m_vulkanResources->m_descriptorPool,
                1,
                &m_sceneDataDescriptorSets[i]);
        }
    }

    m_sceneDataDescriptorSets.clear();
    m_sceneDataDescriptorSets.resize(imageCount);
    std::vector<VkDescriptorSetLayout> layouts(imageCount, m_vulkanResources->m_descriptorSetLayout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vulkanResources->m_descriptorPool;
    allocInfo.descriptorSetCount = imageCount;
    allocInfo.pSetLayouts = layouts.data();

    const auto result = vkAllocateDescriptorSets(
            m_vulkanResources->m_logicalDevice,
            &allocInfo,
            m_sceneDataDescriptorSets.data());

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    std::vector<VkDescriptorImageInfo> imageInfos{m_textures.size()};

    for (size_t i = 0; i < m_textures.size(); i++)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_textures[i]->getImageView();
        imageInfo.sampler = m_sampler;

        imageInfos[i] = imageInfo;
    }

    for (size_t i = 0; i < imageCount; i++)
    {
        const auto set = m_sceneDataDescriptorSets[i];

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_cameraBuffers[i]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(CameraUniformData);

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = set;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = set;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = imageInfos.size();
        descriptorWrites[1].pImageInfo = imageInfos.data();

        vkUpdateDescriptorSets(
            m_vulkanResources->m_logicalDevice,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr);
    }

    return m_textures.size() - 1;
}

void VulkanRenderer::onMeshCreated(const Mesh& mesh)
{
    const auto& vertices = mesh.getVertices();
    const auto& indices = mesh.getIndices();
    const size_t index = mesh.getMeshIndex();

    if (index > m_vertexBuffers.size())
    {
        m_vertexBuffers.resize(m_vertexBuffers.size() * 2);
        m_indexBuffers.resize(m_indexBuffers.size() * 2);
    }

    auto vertexBuffer = std::make_unique<Buffer>(
        m_vulkanResources,
        sizeof(vertices[0]) * vertices.size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vertexBuffer->writeData(vertices.data(), sizeof(vertices[0]) * vertices.size());
    m_vertexBuffers[index] = std::move(vertexBuffer);

    auto indexBuffer = std::make_unique<Buffer>(
        m_vulkanResources,
        sizeof(indices[0]) * indices.size(),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    indexBuffer->writeData(indices.data(), sizeof(indices[0]) * indices.size());
    m_indexBuffers[index] = std::move(indexBuffer);
}

void VulkanRenderer::initializeSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_vulkanResources->m_physicalDevice, &properties);

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    const VkResult result = vkCreateSampler(
        m_vulkanResources->m_logicalDevice,
        &samplerInfo,
        m_vulkanResources->m_allocator,
        &m_sampler);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void VulkanRenderer::updateCamera(const Camera& camera, size_t imageIndex)
{
    const auto constants = CameraUniformData
    {
        camera.getViewProjectionMatrix()
    };

    m_cameraBuffers[imageIndex]->writeData(&constants, sizeof(CameraUniformData));
}

void VulkanRenderer::updateObjectBuffers(
    VkCommandBuffer commandBuffer,
    size_t imageIndex)
{
    m_instanceIndices.clear();

    for (auto& drawRequest : m_drawRequests)
    {
        m_instanceIndices.push_back(drawRequest.instanceIndex);
    }

    m_instanceIndexBuffers[imageIndex]->writeData(m_instanceIndices.data(), sizeof(uint32_t) * m_drawRequests.size());

    for (const auto& data : m_objectBuffers)
    {
        data->updateGpuBuffer(commandBuffer, m_vulkanResources->m_graphicsQueue, imageIndex);
        vkQueueWaitIdle(m_vulkanResources->m_graphicsQueue);
    }
}

void VulkanRenderer::drawScene(
    const Camera& camera,
    const std::vector<DrawRequest>& drawRequests,
    ImDrawData* uiData)
{
    m_drawRequests = drawRequests; // Copy

    std::ranges::stable_sort(m_drawRequests, [](const auto& a, const auto& b) {
        if (a.layer != b.layer)
        {
            return a.layer < b.layer;
        }

        if (a.orderInLayer != b.orderInLayer)
        {
            return a.orderInLayer < b.orderInLayer;
        }

        return false;
    });

    auto swapchain = m_vulkanResources->getSwapchain().lock();
    const auto currentFrameElement = swapchain->getCurrentFrame();

    vkWaitForFences(m_vulkanResources->m_logicalDevice, 1, &currentFrameElement->fence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_vulkanResources->m_logicalDevice,
        swapchain->m_swapchain,
        UINT64_MAX,
        currentFrameElement->startSemaphore,
        VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        swapchain.reset();
        m_vulkanResources->recreateSwapchain();
        return;
    }

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    const auto currentImageElement = swapchain->getFrameAt(imageIndex);

    if (currentImageElement->lastFence)
    {
        const VkResult fenceResult = vkWaitForFences(
            m_vulkanResources->m_logicalDevice,
            1,
            &currentImageElement->lastFence,
            true,
            UINT64_MAX);

        if (fenceResult != VK_SUCCESS)
        {
            throw std::runtime_error("failed to wait for last element fence");
        }
    }

    currentImageElement->lastFence = currentFrameElement->fence;

    vkResetFences(m_vulkanResources->m_logicalDevice, 1, &(currentFrameElement->fence));
    vkResetCommandBuffer(currentImageElement->commandBuffer, 0);

    updateCamera(camera, imageIndex);
    updateObjectBuffers(currentImageElement->commandBuffer, imageIndex);

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(currentImageElement->commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    imageToAttachmentLayout(currentImageElement);

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = currentImageElement->imageView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {
        { 0, 0 },
        { swapchain->m_width, swapchain->m_height }
    };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(currentImageElement->commandBuffer, &renderingInfo);

    // Set render size
    VkViewport viewport = {
        0,
        0,
        static_cast<float>(swapchain->m_width),
        static_cast<float>(swapchain->m_height),
        0,
        1
    };
    vkCmdSetViewport(currentImageElement->commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        { 0, 0 },
        { swapchain->m_width, swapchain->m_height }
    };
    vkCmdSetScissor(currentImageElement->commandBuffer, 0, 1, &scissor);

    const auto currentFrameIndex = swapchain->getCurrentFrameIndex();

    const Mesh& mesh = *m_meshes[0];
    const size_t meshIndex = mesh.getMeshIndex();

    VkBuffer vertexBuffers[] = { m_vertexBuffers[meshIndex]->getBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(currentImageElement->commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(currentImageElement->commandBuffer, m_indexBuffers[meshIndex]->getBuffer(), 0, VK_INDEX_TYPE_UINT16);

    size_t batchStartIndex = 0;
    size_t batchEndIndex = 0;

    // Big loop over all objects to make sure everything gets drawn.
    while (
        batchStartIndex < m_drawRequests.size() &&
        batchEndIndex < m_drawRequests.size())
    {
        const auto& currentBatchStartElement = m_drawRequests[batchStartIndex];

        // Small loop to find the elements for the next draw batch
        while (
            batchEndIndex < m_drawRequests.size() - 1 &&
            m_drawRequests[batchEndIndex + 1].layer == currentBatchStartElement.layer &&
            m_drawRequests[batchEndIndex + 1].objectBufferIndex == currentBatchStartElement.objectBufferIndex &&
            m_drawRequests[batchEndIndex + 1].pipelineIndex == currentBatchStartElement.pipelineIndex)
        {
            batchEndIndex += 1;
        }

        drawIndexed(
            currentImageElement,
            currentFrameIndex,
            currentBatchStartElement.pipelineIndex,
            currentBatchStartElement.objectBufferIndex,
            batchStartIndex,
            batchEndIndex);

        batchStartIndex = batchEndIndex + 1;
        batchEndIndex = batchEndIndex + 1;
    }

    if (uiData)
    {
        ImGui_ImplVulkan_RenderDrawData(uiData, currentImageElement->commandBuffer);
    }

    vkCmdEndRendering(currentImageElement->commandBuffer);

    imageToPresentLayout(currentImageElement);

    if(vkEndCommandBuffer(currentImageElement->commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &currentFrameElement->startSemaphore;
    submitInfo.pWaitDstStageMask = &waitStage;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentImageElement->commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &currentFrameElement->endSemaphore;

    if(vkQueueSubmit(m_vulkanResources->m_graphicsQueue, 1, &submitInfo, currentFrameElement->fence)) {}

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &currentFrameElement->endSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->m_swapchain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_vulkanResources->m_graphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        swapchain.reset();
        m_vulkanResources->recreateSwapchain();
        return;
    }

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present image!");
    }

    swapchain->moveToNextFrame();
}

void VulkanRenderer::imageToAttachmentLayout(SwapchainElement *element)
{
    VkImageMemoryBarrier beforeBarrier{};
    beforeBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    beforeBarrier.srcAccessMask = VK_ACCESS_NONE;
    beforeBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    beforeBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    beforeBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    beforeBarrier.image = element->image;
    beforeBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    beforeBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    beforeBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    vkCmdPipelineBarrier(
        element->commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &beforeBarrier
    );
}

void VulkanRenderer::imageToPresentLayout(SwapchainElement *element)
{
    VkImageMemoryBarrier afterBarrier{};
    afterBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    afterBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    afterBarrier.dstAccessMask = VK_ACCESS_NONE;
    afterBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    afterBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    afterBarrier.image = element->image;
    afterBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    afterBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    afterBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    vkCmdPipelineBarrier(
        element->commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &afterBarrier
    );
}



