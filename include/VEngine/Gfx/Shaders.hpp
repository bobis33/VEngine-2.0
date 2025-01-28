///
/// @file Shaders.hpp
/// @brief This file contains the Shader class
/// @namespace ven
///

#pragma once

#include <string_view>
#include <vector>

#include <vulkan/vulkan_core.h>

namespace ven {

    static constexpr std::string_view SHADERS_BIN_PATH = "build/shaders/";

    ///
    /// @class Shaders
    /// @brief Class for shaders
    /// @namespace ven
    ///
    class Shaders {

        public:

            explicit Shaders(const VkDevice& device) : m_device{device} { }
            ~Shaders() { vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr); }

            Shaders(const Shaders&) = delete;
            Shaders& operator=(const Shaders&) = delete;
            Shaders(Shaders&&) = delete;
            Shaders& operator=(Shaders&&) = delete;

            void createGraphicsPipeline(const VkSampleCountFlagBits& msaaSample, const VkDescriptorSetLayout& descriptorSetLayout, VkPipelineLayout& pipelineLayout, const VkRenderPass& renderPass);

            [[nodiscard]] VkPipeline& getGraphicsPipeline() { return m_graphicsPipeline; }

        private:

            void createShaderModule(const std::vector<char>& code, VkShaderModule& shaderModule) const;

            const VkDevice& m_device;
            VkPipeline m_graphicsPipeline{nullptr};

    }; // class Shaders

} // namespace ven
