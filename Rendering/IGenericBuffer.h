//
// Created by patri on 04.07.2026.
//

#ifndef IGENERICBUFFERINTERFACE_H
#define IGENERICBUFFERINTERFACE_H

class IGenericBuffer
{
public:
    virtual ~IGenericBuffer() = default;
    [[nodiscard]] virtual void* getData() =  0;
    [[nodiscard]] virtual size_t getStride() const = 0;
    [[nodiscard]] virtual size_t getCount() const = 0;
    [[nodiscard]] virtual VkDescriptorSet getDescriptorSet(size_t imageIndex) const = 0;
    [[nodiscard]] size_t getTotalSize() const { return getStride() * getCount(); }

    virtual void updateGpuBuffer(
        VkCommandBuffer commandBuffer,
        VkQueue queue,
        size_t imageIndex) const = 0;
};

#endif //IGENERICBUFFERINTERFACE_H
