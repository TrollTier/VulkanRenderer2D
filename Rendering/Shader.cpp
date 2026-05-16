//
// Created by patri on 10.05.2026.
//

#include "Shader.h"
#include <vector>
#include <fstream>

static std::vector<char> readFile(const std::filesystem::path& fileName) {
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

static VkShaderModule createShaderModule(VkDevice device, const std::filesystem::path& filePath)
{
    auto code = readFile(filePath);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    VkShaderModule module;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &module) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }

    return module;
}

Shader::Shader(
    VkDevice device,
    const std::filesystem::path &vertexShaderPath,
    const std::filesystem::path &fragmentShaderPath)
{
    m_device = device;
    m_fragmentShaderModule = createShaderModule(device, fragmentShaderPath);
    m_vertexShaderModule = createShaderModule(device, vertexShaderPath);
}

Shader::~Shader()
{
    vkDestroyShaderModule(m_device, m_fragmentShaderModule, nullptr);
    vkDestroyShaderModule(m_device, m_vertexShaderModule, nullptr);
}

VkShaderModule Shader::getFragmentShaderModule() const
{
    return m_fragmentShaderModule;
}

VkShaderModule Shader::getVertexShaderModule() const
{
    return m_vertexShaderModule;
}

