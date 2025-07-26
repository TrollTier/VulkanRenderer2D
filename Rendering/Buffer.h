//
// Created by patri on 26.07.2025.
//

#ifndef BUFFER_H
#define BUFFER_H

#include <memory>

#include "VulkanRessources.h"
#include  "vulkan/vulkan.h"

class Buffer
{
public:
    Buffer(
        const std::weak_ptr<VulkanRessources> &resources,
        uint32_t size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties);
    ~Buffer();

    [[nodiscard]] VkBuffer getBuffer() const { return m_buffer; }
    [[nodiscard]] VkDeviceMemory getBufferMemory() const { return m_bufferMemory; }
    [[nodiscard]] const void* getBufferMappedMemory() const { return m_bufferMemoryMapped; }

private:
    std::weak_ptr<VulkanRessources> m_resources;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
    void* m_bufferMemoryMapped = VK_NULL_HANDLE;
};

#endif //BUFFER_H
