#include "VulkanRenderer.h"

#include <stdexcept>
#include "VulkanHelpers.h"
#include <fstream>
#include "Vertex.h"
#include "Sampler.h"
#include "Descriptors.h"
#include "Instance.h"
#include "Swapchain.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <ranges>

#include "VulkanRessources.h"
#include "VulkanWindow.h"
#include "../include/glfw-3.4/include/GLFW/glfw3native.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb/stb_image.h"

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
};

const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0
};

VulkanRenderer::~VulkanRenderer()
{
    const auto device = m_vulkanRessources->m_logicalDevice;
    const auto allocator = m_vulkanRessources->m_allocator;

    vkDeviceWaitIdle(device);

    vkFreeMemory(device, m_indexBufferMemory, allocator);
    vkDestroyBuffer(device, m_indexBuffer, allocator);

    vkFreeMemory(device, m_vertexBufferMemory, allocator);
    vkDestroyBuffer(device, m_vertexBuffer, allocator);

    vkDestroyDescriptorPool(device, m_descriptorPool, allocator);
    vkDestroySampler(device, m_sampler, allocator);
    vkDestroyPipeline(device, m_graphicsPipeline, allocator);
    vkDestroyPipelineLayout(device, m_pipelineLayout, allocator);
    vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, allocator);
}

static std::vector<char> readFile(const std::string& fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    VkShaderModule module;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }

    return module;
}

void VulkanRenderer::initialize(
    bool enableValidationLayers,
    VulkanWindow& window)
{
    std::vector<const char*> instanceExtensions{};
    window.fillRequiredInstanceExtensions(instanceExtensions);
    instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

    m_vulkanRessources = std::make_shared<VulkanRessources>();
    m_vulkanRessources->initialize(
        enableValidationLayers,
        m_validationLayers,
        instanceExtensions,
        window);

    m_swapchain = std::make_unique<Swapchain>(m_vulkanRessources);

    initializeDescriptorSetLayout(
        m_vulkanRessources->m_logicalDevice,
        m_descriptorSetLayout,
        m_vulkanRessources->m_allocator);

    initializeDescriptorPool(
        m_vulkanRessources->m_physicalDevice,
        m_vulkanRessources->m_logicalDevice,
        MAX_FRAMES_IN_FLIGHT,
        m_descriptorPool,
        m_vulkanRessources->m_allocator);

    std::vector<VkDescriptorSetLayout> layouts(2, m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 2;
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(allocInfo.descriptorSetCount);

    if (vkAllocateDescriptorSets(m_vulkanRessources->m_logicalDevice, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    const size_t imageCount = m_swapchain->getImageCount();
    m_uniformBuffers.resize(imageCount);
    m_uniformBufferMemories.resize(imageCount);
    m_uniformBuffersMapped.resize(imageCount);

    for (size_t i = 0; i < imageCount; i++) {
        createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            m_uniformBuffers[i],
            m_uniformBufferMemories[i]);

        vkMapMemory(
            m_vulkanRessources->m_logicalDevice,
            m_uniformBufferMemories[i],
            0,
            bufferSize,
            0,
            &m_uniformBuffersMapped[i]);
    }

    VkPipelineShaderStageCreateInfo inputAssemblerCreateInfo = {};
    inputAssemblerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    auto vertShaderCode = readFile("../Shaders/vert.spv");
    auto fragShaderCode = readFile("../Shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(m_vulkanRessources->m_logicalDevice, vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(m_vulkanRessources->m_logicalDevice, fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    const VkVertexInputBindingDescription binding = Vertex::getBindingDescription();
    const auto attributes = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &binding;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();

    VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Dynamic states
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.setLayoutCount = m_descriptorSets.size();
    pipelineLayoutInfo.pSetLayouts = layouts.data();

    if (vkCreatePipelineLayout(
            m_vulkanRessources->m_logicalDevice,
            &pipelineLayoutInfo,
            m_vulkanRessources->m_allocator,
            &m_pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    const auto colorFormat = m_swapchain->m_format.format;
    VkPipelineRenderingCreateInfo renderingInfo{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachmentFormats = &colorFormat;

    VkPipelineShaderStageCreateInfo stages[] = { vertShaderStageInfo, fragShaderStageInfo };
    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.pNext = &renderingInfo;
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = stages;
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizer;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssembly;
    graphicsPipelineCreateInfo.pViewportState = &viewportState;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlending;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicState;
    graphicsPipelineCreateInfo.layout = m_pipelineLayout;
    graphicsPipelineCreateInfo.renderPass = VK_NULL_HANDLE; // because we're using dynamic rendering
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampling;

    if (vkCreateGraphicsPipelines(
            m_vulkanRessources->m_logicalDevice,
            nullptr,
            1,
            &graphicsPipelineCreateInfo,
            m_vulkanRessources->m_allocator,
            &m_graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_vulkanRessources->m_logicalDevice, vertShaderModule, m_vulkanRessources->m_allocator);
    vkDestroyShaderModule(m_vulkanRessources->m_logicalDevice, fragShaderModule, m_vulkanRessources->m_allocator);

    initializeSampler(
        m_vulkanRessources->m_physicalDevice,
        m_vulkanRessources->m_logicalDevice,
        &m_sampler,
        m_vulkanRessources->m_allocator);

    createBufferWithData(
        vertices.data(),
        sizeof(vertices[0]) * vertices.size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        m_vertexBuffer,
        m_vertexBufferMemory);

    createBufferWithData(
        indices.data(),
        sizeof(indices[0]) * indices.size(),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        m_indexBuffer,
        m_indexBufferMemory);


    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load("../Assets/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("Failed to load texture image!");
    }

    createImage(
        texWidth,
        texHeight,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_textureImage,
        m_textureImageMemory);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_vulkanRessources->m_logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (imageSize));
    vkUnmapMemory(m_vulkanRessources->m_logicalDevice, stagingBufferMemory);

    stbi_image_free(pixels);

    transitionImageLayout(
        m_textureImage,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(stagingBuffer, m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    transitionImageLayout(
        m_textureImage,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image = m_textureImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_vulkanRessources->m_logicalDevice, &viewInfo, nullptr, &m_textureImageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create texture image view!");
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_textureImageView;
    imageInfo.sampler = m_sampler;

    for (size_t i = 0; i < m_swapchain->getImageCount(); i++)
    {
        const auto set = m_descriptorSets[i];

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

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
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(
            m_vulkanRessources->m_logicalDevice,
            static_cast<uint32_t>(descriptorWrites.size()),
            descriptorWrites.data(),
            0,
            nullptr);
    }
}

void VulkanRenderer::createBufferWithData(
    const void *srcData,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkBuffer &dstBuffer,
    VkDeviceMemory &dstBufferMemory)
{
    const VkDevice device = m_vulkanRessources->m_logicalDevice;
    const VkQueue transferQueue = m_vulkanRessources->m_graphicsQueue;
    const VkCommandPool commandPool = m_vulkanRessources->m_commandPool;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    createBuffer(size,
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
    createBuffer(size,
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


void VulkanRenderer::createBuffer(VkDeviceSize size,
                  VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties,
                  VkBuffer& buffer,
                  VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_vulkanRessources->m_logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_vulkanRessources->m_logicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_vulkanRessources->m_logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate buffer memory");
    }

    if (vkBindBufferMemory(m_vulkanRessources->m_logicalDevice, buffer, bufferMemory, 0) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to bind buffer memory");
    }
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vulkanRessources->m_physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        auto maskedProperties = (memoryProperties.memoryTypes[i].propertyFlags & properties);

        if (typeFilter & (1 << i) && maskedProperties == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}

void VulkanRenderer::createImage(
    uint32_t width,
    uint32_t height,
    VkFormat format,
    VkImageTiling tiling,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkImage& image,
    VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0;

    if (vkCreateImage(m_vulkanRessources->m_logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_vulkanRessources->m_logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        memRequirements.memoryTypeBits,
        properties
    );

    if (vkAllocateMemory(m_vulkanRessources->m_logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate image memory!");
    }

    vkBindImageMemory(m_vulkanRessources->m_logicalDevice, image, imageMemory, 0);
}

void VulkanRenderer::transitionImageLayout(
    VkImage image,
    VkFormat format,
    VkImageLayout oldLayout,
    VkImageLayout newLayout)
{
    const VkCommandPool commandPool = m_vulkanRessources->m_commandPool;
    const VkQueue graphicsQueue = m_vulkanRessources->m_graphicsQueue;
    const VkDevice device = m_vulkanRessources->m_logicalDevice;

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanRenderer::copyBufferToImage(
    VkBuffer buffer,
    VkImage image,
    uint32_t width,
    uint32_t height)
{
    const VkCommandPool commandPool = m_vulkanRessources->m_commandPool;
    const VkQueue graphicsQueue = m_vulkanRessources->m_graphicsQueue;
    const VkDevice device = m_vulkanRessources->m_logicalDevice;

    VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;     // tightly packed
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanRenderer::updateUniformBuffer(size_t imageIndex) {
    float time = 0;

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.view = glm::lookAt(
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.projection = glm::perspective(
        glm::radians(45.0f),
        m_swapchain->m_width / (float)m_swapchain->m_height,
        0.1f,
        10.0f);
    ubo.projection[1][1] *= -1;

    memcpy(m_uniformBuffersMapped[imageIndex], &ubo, sizeof(ubo));
}

void VulkanRenderer::draw_scene()
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

    // TODO: recreate swapchain if required
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

    VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(currentImageElement->commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    updateUniformBuffer(imageIndex);

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
        m_graphicsPipeline);

    VkBuffer vertexBuffers[] = { m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(currentImageElement->commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(currentImageElement->commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // Bind global descriptor set
    vkCmdBindDescriptorSets(
        currentImageElement->commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipelineLayout,
        0,
        1,
        &m_descriptorSets[m_swapchain->getCurrentFrameIndex()],
        0,
        nullptr
    );

    vkCmdDrawIndexed(
        currentImageElement->commandBuffer,
        static_cast<uint32_t>(indices.size()),
        1,
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

    // TODO: recreate swapchain logic for resize or similar
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


