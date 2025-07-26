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
};

#endif //INSTANCEDATA_H
