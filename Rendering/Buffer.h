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
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties);
    ~Buffer();

    [[nodiscard]] VkBuffer getBuffer() const { return m_buffer; }
    [[nodiscard]] VkDeviceMemory getBufferMemory() const { return m_bufferMemory; }
    [[nodiscard]] VkDeviceSize getSize() const { return m_bufferSize;}

    [[nodiscard]] void* mapMemory(VkDeviceSize length) const;
    void unmapMemory() const;

    void writeData(const void* data, VkDeviceSize length) const;

private:
    std::weak_ptr<VulkanRessources> m_resources;
    VkBuffer m_buffer = VK_NULL_HANDLE;
    VkDeviceMemory m_bufferMemory = VK_NULL_HANDLE;
    VkDeviceSize m_bufferSize = 0;
};

#endif //BUFFER_H
