//
// Created by patri on 29.06.2026.
//

#ifndef OBJECTBUFFER_H
#define OBJECTBUFFER_H

#include <vector>
#include "Buffer.h"

template <typename T>
class ObjectBuffer {

public:
    std::vector<std::unique_ptr<Buffer>> m_objectStagingBuffers{};
    std::vector<std::unique_ptr<Buffer>> m_objectBuffers{};
    std::vector<VkDescriptorSet> m_objectBufferDescriptors{};
    std::vector<T> m_data{};
    size_t m_dataSize = 0;

    ObjectBuffer(
        const std::weak_ptr<VulkanResources>& vulkanResources,
        size_t images,
        size_t bufferSize)
    {
        m_vulkanResources = vulkanResources;
        m_images = images;
        m_data.resize(bufferSize);
        m_objectBufferDescriptors.reserve(images);
        m_objectBuffers.reserve(images);
        m_objectStagingBuffers.reserve(images);

        if (m_vulkanResources.expired())
        {
            return;
        }

        const auto resources = m_vulkanResources.lock();

        const auto objectBufferLayout = resources->m_descriptorSetLayoutObjectsBuffer;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{images, objectBufferLayout};

        VkDescriptorSetAllocateInfo objectBufferInfo = {};
        objectBufferInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        objectBufferInfo.descriptorSetCount = images;
        objectBufferInfo.descriptorPool = resources->m_descriptorPool;
        objectBufferInfo.pSetLayouts = descriptorSetLayouts.data();

        const VkResult result = vkAllocateDescriptorSets(
            resources->m_logicalDevice,
            &objectBufferInfo,
            m_objectBufferDescriptors.data());

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate object buffer descriptor sets!");
        }

        for (size_t i = 0; i < images; i++)
        {
            const auto size = sizeof(T) * bufferSize;

            m_objectStagingBuffers.emplace_back(
                std::make_unique<Buffer>(
                    vulkanResources,
                    size,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

            m_objectBuffers.emplace_back(
                std::make_unique<Buffer>(
                    m_vulkanResources,
                    size,
                    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_objectBuffers[i]->getBuffer();
            bufferInfo.offset = 0;
            bufferInfo.range = size;

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.dstSet = m_objectBufferDescriptors[i];
            writeDescriptorSet.dstBinding = 0;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(
                resources->m_logicalDevice,
                1,
                &writeDescriptorSet,
                0,
                nullptr);
        }
    }

    ~ObjectBuffer()
    {
        if (m_vulkanResources.expired())
        {
            return;
        }

        const auto resources = m_vulkanResources.lock();

        vkFreeDescriptorSets(
            resources->m_logicalDevice,
            resources->m_descriptorPool,
            m_objectBufferDescriptors.size(),
            m_objectBufferDescriptors.data());

        m_objectBufferDescriptors.clear();
        m_data.clear();
    }

    void updateGpuBuffer(
        VkCommandBuffer commandBuffer,
        VkQueue queue,
        size_t imageIndex) const
    {
        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        const auto& objectBuffer = m_objectBuffers[imageIndex];
        const auto& stagingBuffer = *m_objectStagingBuffers[imageIndex];
        const auto& data = m_data;

        const auto stagingBufferSize = sizeof(T) * m_dataSize;
        stagingBuffer.writeData(data.data(), stagingBufferSize);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = stagingBufferSize;

        vkCmdCopyBuffer(commandBuffer, stagingBuffer.getBuffer(), objectBuffer->getBuffer(), 1, &copyRegion);
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    }

    void append(T data)
    {
        if (m_data.capacity() < m_dataSize + 1)
        {
            m_data.resize(m_data.capacity() * 2);
        }

        m_data[m_dataSize++] = data;
    }

private:
    std::weak_ptr<VulkanResources> m_vulkanResources;
    size_t m_images;
};



#endif //OBJECTBUFFER_H
