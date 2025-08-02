#include "VulkanRenderer.h"
#include <stdexcept>
#include "VulkanHelpers.h"
#include <fstream>
#include "../Core/Vertex.h"
#include "Swapchain.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <iostream>

#include "CameraUniformData.h"
#include "VulkanRessources.h"
#include "VulkanWindow.h"
#include "GLFW/glfw3native.h"

#include "InstanceData.h"

VulkanRenderer::~VulkanRenderer()
{
    const auto device = m_vulkanRessources->m_logicalDevice;
    const auto allocator = m_vulkanRessources->m_allocator;
    const auto& descriptorPool =  m_pipeline->getDescriptorPool();

    vkDeviceWaitIdle(device);

    m_objectBuffers.clear();
    m_cameraBuffers.clear();

    for (size_t i = 0; i < m_indexBuffers.size(); i++)
    {
        vkFreeMemory(device, m_indexBufferMemories[i], allocator);
        vkDestroyBuffer(device, m_indexBuffers[i], allocator);

        vkFreeMemory(device, m_vertexBufferMemories[i], allocator);
        vkDestroyBuffer(device, m_vertexBuffers[i], allocator);
    }

    vkDestroySampler(device, m_sampler, allocator);
}

void VulkanRenderer::initialize(
    bool enableValidationLayers,
    std::shared_ptr<VulkanWindow> window)
{
    std::vector<const char*> instanceExtensions{};
    window->fillRequiredInstanceExtensions(instanceExtensions);
    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    m_vulkanRessources = std::make_shared<VulkanRessources>(window);
    m_vulkanRessources->initialize(
        enableValidationLayers,
        m_validationLayers,
        instanceExtensions);

    m_swapchain = std::make_unique<Swapchain>(m_vulkanRessources);
    m_pipeline = std::make_unique<Pipeline>(
        m_vulkanRessources,
        "../Shaders/vert.spv",
        "../Shaders/frag.spv",
        m_swapchain->getImageCount(),
        m_swapchain->m_format.format);

    initializeSampler();
    initializeDefaultMeshes();

    const auto imageCount = m_swapchain->getImageCount();
    m_cameraBuffers.reserve(imageCount);
    m_defaultDescriptorSets.resize(imageCount);
    m_objectBufferDescriptors.resize(imageCount);

    const auto objectBufferLayout = m_pipeline->getDescriptorSetLayoutObjectsBuffer();
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{imageCount, objectBufferLayout};

    VkDescriptorSetAllocateInfo objectBufferInfo = {};
    objectBufferInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    objectBufferInfo.descriptorSetCount = imageCount;
    objectBufferInfo.descriptorPool = m_pipeline->getDescriptorPool();
    objectBufferInfo.pSetLayouts = descriptorSetLayouts.data();

    const VkResult result = vkAllocateDescriptorSets(
            m_vulkanRessources->m_logicalDevice,
            &objectBufferInfo,
            m_objectBufferDescriptors.data());

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate object buffer descriptor sets!");
    }

    for (int i = 0; i < imageCount; i++)
    {
        m_cameraBuffers.emplace_back(
            std::make_unique<Buffer>(
                m_vulkanRessources,
                sizeof(CameraUniformData),
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

        m_objectBuffers.emplace_back(
            std::make_unique<Buffer>(
                m_vulkanRessources,
                sizeof(InstanceData) * 10000,
                VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_objectBuffers[i]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(InstanceData) * 10000;

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = m_objectBufferDescriptors[i];
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(
            m_vulkanRessources->m_logicalDevice,
            1,
            &writeDescriptorSet,
            0,
            nullptr);
    }
}

void VulkanRenderer::initializeDefaultMeshes()
{
    const std::vector<Vertex> vertices = {
        {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    m_meshes.emplace_back(std::make_unique<Mesh>(0, vertices, indices));
    onMeshCreated(*m_meshes[m_meshes.size() - 1]);
}

size_t VulkanRenderer::loadTexture(const char *texturePath)
{
    m_textures.emplace_back(std::make_unique<Texture2D>(m_vulkanRessources, texturePath));

    const size_t imageCount =  m_swapchain->getImageCount();

    for (size_t i = 0; i < imageCount; i++)
    {
        if (m_defaultDescriptorSets[i] != VK_NULL_HANDLE)
        {
            vkFreeDescriptorSets(
                m_vulkanRessources->m_logicalDevice,
                m_pipeline->getDescriptorPool(),
                1,
                &m_defaultDescriptorSets[i]);
        }
    }

    m_defaultDescriptorSets.clear();
    m_defaultDescriptorSets.resize(imageCount);
    std::vector<VkDescriptorSetLayout> layouts(imageCount, m_pipeline->getDescriptorSetLayout());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_pipeline->getDescriptorPool();
    allocInfo.descriptorSetCount = imageCount;
    allocInfo.pSetLayouts = layouts.data();

    const auto result = vkAllocateDescriptorSets(
            m_vulkanRessources->m_logicalDevice,
            &allocInfo,
            m_defaultDescriptorSets.data());

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
        const auto set = m_defaultDescriptorSets[i];

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
            m_vulkanRessources->m_logicalDevice,
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

    if (index >= m_indexBuffers.size())
    {
        m_indexBuffers.resize(m_indexBuffers.size() * 2, VK_NULL_HANDLE);
        m_indexBufferMemories.resize(m_indexBufferMemories.size() * 2, VK_NULL_HANDLE);
        m_vertexBuffers.resize(m_vertexBuffers.size() * 2, VK_NULL_HANDLE);
        m_vertexBufferMemories.resize(m_vertexBufferMemories.size() * 2, VK_NULL_HANDLE);
    }

    createBufferWithData(
        vertices.data(),
        sizeof(vertices[0]) * vertices.size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        m_vertexBuffers[index],
        m_vertexBufferMemories[index]);

    createBufferWithData(
        indices.data(),
        sizeof(indices[0]) * indices.size(),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        m_indexBuffers[index],
        m_indexBufferMemories[index]);
}

void VulkanRenderer::initializeSampler()
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_vulkanRessources->m_physicalDevice, &properties);

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
        m_vulkanRessources->m_logicalDevice,
        &samplerInfo,
        m_vulkanRessources->m_allocator,
        &m_sampler);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void VulkanRenderer::createBufferWithData(
    const void *srcData,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkBuffer &dstBuffer,
    VkDeviceMemory &dstBufferMemory)
{
    VkDevice device = m_vulkanRessources->m_logicalDevice;
    VkQueue transferQueue = m_vulkanRessources->m_graphicsQueue;
    VkCommandPool commandPool = m_vulkanRessources->m_commandPool;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VulkanHelpers::createBuffer(
        m_vulkanRessources->m_logicalDevice,
        m_vulkanRessources->m_physicalDevice,
        size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    // Copy data to staging buffer
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, size, 0, &data);
    memcpy(data, srcData, static_cast<size_t>(size));
    vkUnmapMemory(device, stagingBufferMemory);

    // --- 2. Create GPU-local destination buffer
    VulkanHelpers::createBuffer(
        m_vulkanRessources->m_logicalDevice,
        m_vulkanRessources->m_physicalDevice,
        size,
        usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        dstBuffer, dstBufferMemory);

    // --- 3. Copy buffer using command buffer
    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmdBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &cmdBuffer);

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;

    vkCmdCopyBuffer(cmdBuffer, stagingBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(transferQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &cmdBuffer);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanRenderer::updateCamera(size_t imageIndex)
{
    glm::vec3 eye    = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up     = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(eye, center, up);

    glm::mat4 projection = glm::ortho(
        0.0f,
        (float)m_swapchain->m_width,
        0.0f,
        (float)m_swapchain->m_height);

    const auto constants = CameraUniformData
    {
        projection * view
    };

    memcpy(
        (void*)m_cameraBuffers[imageIndex]->getBufferMappedMemory(),
        &constants,
        sizeof(CameraUniformData));
}

void VulkanRenderer::updateObjectsBuffer(VkCommandBuffer commandBuffer, size_t imageIndex, const Map &map, const World &world)
{
    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    const auto& objectBuffer = m_objectBuffers[imageIndex];

    const auto tileSize = map.getTileSize();
    const auto& tiles = map.getTiles();
    const auto& gameObjects = world.getGameObjects();

    std::vector<InstanceData> objectSSBO(tiles.size() + gameObjects.size());

    for (int i = 0; i < tiles.size(); i++)
    {
        const auto tile = tiles[i];
        const glm::vec3 worldPos = glm::vec3(tile.column * tileSize, tile.row * tileSize, 0);

        objectSSBO[i].modelMatrix =
            glm::translate(glm::mat4(1.0f), worldPos) *
            glm::scale(glm::mat4(1), glm::vec3(64.0f, 64.0f, 1.0f));
        objectSSBO[i].textureIndex = tile.sprite.textureIndex;
    }

    const auto objectsOffset = tiles.size();

    for (int i = 0; i < gameObjects.size(); i++)
    {
        const auto gameObject = gameObjects[i];

        objectSSBO[i + objectsOffset].modelMatrix =
            glm::translate(glm::mat4(1.0f), gameObject.getWorldPosition()) *
            glm::scale(glm::mat4(1), glm::vec3(64.0f, 64.0f, 1.0f));
        objectSSBO[i + objectsOffset].textureIndex = gameObject.getSprite().textureIndex;
    }

    const auto stagingBufferSize = sizeof(InstanceData) * objectSSBO.size();
    Buffer stagingBuffer(
        m_vulkanRessources,
        stagingBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    // Copy data to staging buffer
    memcpy(stagingBuffer.getBufferMappedMemoryWritable(), objectSSBO.data(), stagingBufferSize);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = stagingBufferSize;

    vkCmdCopyBuffer(commandBuffer, stagingBuffer.getBuffer(), objectBuffer->getBuffer(), 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_vulkanRessources->m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vulkanRessources->m_graphicsQueue);
}


void VulkanRenderer::draw_scene(const Map& map, const World& world)
{
    const auto currentFrameElement = m_swapchain->getCurrentFrame();

    vkWaitForFences(m_vulkanRessources->m_logicalDevice, 1, &currentFrameElement->fence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_vulkanRessources->m_logicalDevice,
        m_swapchain->m_swapchain,
        UINT64_MAX,
        currentFrameElement->startSemaphore,
        VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        vkDeviceWaitIdle(m_vulkanRessources->m_logicalDevice);

        m_swapchain.reset();
        m_swapchain = std::make_unique<Swapchain>(m_vulkanRessources);
        return;
    }

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    const auto currentImageElement = m_swapchain->getFrameAt(imageIndex);

    if (currentImageElement->lastFence)
    {
        const VkResult fenceResult = vkWaitForFences(
            m_vulkanRessources->m_logicalDevice,
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

    vkResetFences(m_vulkanRessources->m_logicalDevice, 1, &(currentFrameElement->fence));

    vkResetCommandBuffer(currentImageElement->commandBuffer, 0);

    updateCamera(imageIndex);
    updateObjectsBuffer(currentImageElement->commandBuffer, imageIndex, map, world);

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
        { m_swapchain->m_width, m_swapchain->m_height }
    };
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(currentImageElement->commandBuffer, &renderingInfo);

    // Set render size
    VkViewport viewport = {
        0,
        0,
        static_cast<float>(m_swapchain->m_width),
        static_cast<float>(m_swapchain->m_height),
        0,
        1
    };
    vkCmdSetViewport(currentImageElement->commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        { 0, 0 },
        { m_swapchain->m_width, m_swapchain->m_height }
    };
    vkCmdSetScissor(currentImageElement->commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(
        currentImageElement->commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipeline->getPipeline());

    std::vector<VkDescriptorSet> descriptorSets{};
    descriptorSets.push_back(m_defaultDescriptorSets[m_swapchain->getCurrentFrameIndex()]);
    descriptorSets.push_back(m_objectBufferDescriptors[m_swapchain->getCurrentFrameIndex()]);

    // Bind global descriptor set
    vkCmdBindDescriptorSets(
        currentImageElement->commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipeline->getLayout(),
        0,
        descriptorSets.size(),
        descriptorSets.data(),
        0,
        nullptr
    );

    const Mesh& mesh = *m_meshes[0];

    const size_t meshIndex = mesh.getMeshIndex();
    VkBuffer vertexBuffers[] = { m_vertexBuffers[meshIndex] };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(currentImageElement->commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(currentImageElement->commandBuffer, m_indexBuffers[meshIndex], 0, VK_INDEX_TYPE_UINT16);

    const auto indices = mesh.getIndices().size();
    const auto objectsCount  = map.getTiles().size() + world.getGameObjects().size();

    vkCmdDrawIndexed(
        currentImageElement->commandBuffer,
        static_cast<uint32_t>(indices),
        objectsCount,
        0,
        0,
        0);

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

    if(vkQueueSubmit(m_vulkanRessources->m_graphicsQueue, 1, &submitInfo, currentFrameElement->fence));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &currentFrameElement->endSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_swapchain->m_swapchain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_vulkanRessources->m_graphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        vkDeviceWaitIdle(m_vulkanRessources->m_logicalDevice);

        m_swapchain.reset();
        m_swapchain = std::make_unique<Swapchain>(m_vulkanRessources);
        return;
    }

    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present image!");
    }

    m_swapchain->moveToNextFrame();
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


