//
// Created by patri on 26.07.2025.
//

#include "Buffer.h"

#include "VulkanHelpers.h"

Buffer::Buffer(
    const std::weak_ptr<VulkanRessources> &resources,
    const uint32_t size,
    const VkBufferUsageFlags usage,
    const VkMemoryPropertyFlags properties)
{
    m_resources = resources;

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

        vkMapMemory(
            ptr->m_logicalDevice,
            m_bufferMemory,
            0,
            size,
            0,
            &m_bufferMemoryMapped);
    }
}

Buffer::~Buffer()
{
    if (const auto ptr = m_resources.lock())
    {
        vkUnmapMemory(ptr->m_logicalDevice, m_bufferMemory);
        vkFreeMemory(ptr->m_logicalDevice, m_bufferMemory, ptr->m_allocator);
        vkDestroyBuffer(ptr->m_logicalDevice, m_buffer, ptr->m_allocator);
    }
}
