//
// Created by patri on 10.05.2026.
//

#ifndef SHADER_H
#define SHADER_H

#include<filesystem>
#include <vulkan/vulkan.h>

class Shader
{
public:
    Shader(
        VkDevice device,
        const std::filesystem::path& vertexShaderPath,
        const std::filesystem::path& fragmentShaderPath);
    ~Shader();

    [[nodiscard]] VkShaderModule getFragmentShaderModule() const;
    [[nodiscard]] VkShaderModule getVertexShaderModule() const;

private:
    VkDevice m_device;
    VkShaderModule m_fragmentShaderModule;
    VkShaderModule m_vertexShaderModule;
};

#endif //SHADER_H
