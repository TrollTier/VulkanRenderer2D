//
// Created by patri on 19.07.2025.
//

#ifndef INSTANCEDATA_H
#define INSTANCEDATA_H

#include <vector>
#include <vulkan/vulkan.h>

struct InstanceData
{
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBufferMemories;
    std::vector<void*> uniformBuffersMapped;
};

#endif //INSTANCEDATA_H
