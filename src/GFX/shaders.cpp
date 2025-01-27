#include <vulkan/vulkan_core.h>

#include "Utils/ErrorHandling.hpp"
#include "VEngine/GFX/Shaders.hpp"

void ven::Shaders::createShaderModule(const VkDevice& device, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    if (vkCreateShaderModule(device, &createInfo, nullptr, &m_shaderModule) != VK_SUCCESS) {
        utl::THROW_ERROR("failed to create shader module");
    }
}
