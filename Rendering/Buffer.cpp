//
// Created by patri on 26.07.2025.
//

#include "Buffer.h"

#include "VulkanHelpers.h"

Buffer::Buffer(
    const std::weak_ptr<VulkanRessources> &resources,
    const VkDeviceSize size,
    const VkBufferUsageFlags usage,
    const VkMemoryPropertyFlags properties)
{
    m_resources = resources;
    m_bufferSize = size;

    if (const auto ptr = resources.lock())
    {
        VulkanHelpers::createBuffer(
            ptr->m_logicalDevice,
            ptr->m_physicalDevice,
            size,
            usage,
            properties,
            m_buffer,
            m_bufferMemory);
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
    void* mappedMemory = mapMemory(length);
    memcpy(mappedMemory, data, length);
    unmapMemory();
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

    return std::move(mappedMemory);
}

void Buffer::unmapMemory() const
{
    if (!m_resources.expired())
    {
        return;
    }

    if (const auto ptr = m_resources.lock())
    {
        vkUnmapMemory(ptr->m_logicalDevice, m_bufferMemory);
    }
}


