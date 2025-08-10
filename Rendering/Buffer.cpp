//
// Created by patri on 26.07.2025.
//

#include "Buffer.h"

#include "VulkanHelpers.h"

Buffer::Buffer(
    const std::weak_ptr<VulkanResources> &resources,
    const VkDeviceSize size,
    const VkBufferUsageFlags usage,
    const VkMemoryPropertyFlags properties)
{
    m_resources = resources;
    m_bufferSize = size;
    m_properties = properties;

    if (const auto ptr = resources.lock())
    {
        VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(ptr->m_logicalDevice, &bufferInfo, nullptr, &m_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(ptr->m_logicalDevice, m_buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VulkanHelpers::findMemoryType(
            ptr->m_physicalDevice,
            memRequirements.memoryTypeBits,
            properties);

        if (vkAllocateMemory(ptr->m_logicalDevice, &allocInfo, nullptr, &m_bufferMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate buffer memory");
        }

        if (vkBindBufferMemory(ptr->m_logicalDevice, m_buffer, m_bufferMemory, 0) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to bind buffer memory");
        }
    }
}

Buffer::~Buffer()
{
    if (const auto ptr = m_resources.lock())
    {
        vkFreeMemory(ptr->m_logicalDevice, m_bufferMemory, ptr->m_allocator);
        vkDestroyBuffer(ptr->m_logicalDevice, m_buffer, ptr->m_allocator);
    }
}

void Buffer::writeData(const void *data, VkDeviceSize length) const
{
    if (m_properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
    {
        void* mappedMemory = mapMemory(length);
        memcpy(mappedMemory, data, length);
        unmapMemory();
    }
    else if (const auto ptr = m_resources.lock())
    {
        Buffer stagingBuffer{
            m_resources,
            length,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

        stagingBuffer.writeData(data, length);

        VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
        allocInfo.commandPool = ptr->m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmdBuffer;
        vkAllocateCommandBuffers(ptr->m_logicalDevice, &allocInfo, &cmdBuffer);

        VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmdBuffer, &beginInfo);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = length;

        vkCmdCopyBuffer(cmdBuffer, stagingBuffer.getBuffer(), m_buffer, 1, &copyRegion);
        vkEndCommandBuffer(cmdBuffer);

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmdBuffer;

        vkQueueSubmit(ptr->m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(ptr->m_graphicsQueue);
    }
}

void* Buffer::mapMemory(VkDeviceSize length) const
{
    void* mappedMemory = nullptr;

    if (m_resources.expired())
    {
        return mappedMemory;
    }

    if (const auto ptr = m_resources.lock())
    {
        const VkResult result = vkMapMemory(
            ptr->m_logicalDevice,
            m_bufferMemory,
            0,
            length,
            0,
            &mappedMemory);

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to map memory");
        }
    }

    return mappedMemory;
}

void Buffer::unmapMemory() const
{
    if (m_resources.expired())
    {
        return;
    }

    if (const auto ptr = m_resources.lock())
    {
        vkUnmapMemory(ptr->m_logicalDevice, m_bufferMemory);
    }
}


