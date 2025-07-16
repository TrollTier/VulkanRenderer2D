//
// Created by patri on 07.07.2025.
//

#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

#include <vulkan/vulkan.h>
#include <stdexcept>

static void initializeDescriptorSetLayout(
    VkDevice device,
    VkDescriptorSetLayout& descriptorSetLayout,
    VkAllocationCallbacks* allocator)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 1;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.pImmutableSamplers = nullptr;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerBinding };

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &createInfo, allocator, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout");
    }
}

static void initializeDescriptorPool(
    VkPhysicalDevice physicalDevice,
    VkDevice logicalDevice,
    uint32_t maxSets,
    VkDescriptorPool& descriptorPool,
    VkAllocationCallbacks* allocator)
{
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = maxSets;
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = maxSets;

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.maxSets = maxSets;
    descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPoolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(logicalDevice, &descriptorPoolInfo, allocator, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool");
    }
}

#endif //DESCRIPTORS_H
