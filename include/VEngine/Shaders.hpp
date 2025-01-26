///
/// @file Shaders.hpp
/// @brief This file contains the Shader class
/// @namespace ven
///

#pragma once

#include <vector>

namespace ven {

    static constexpr std::string_view SHADERS_BIN_PATH = "build/shaders/";

    ///
    /// @class Shaders
    /// @brief Class for shaders
    /// @namespace ven
    ///
    class Shaders {

        public:

            Shaders(const VkDevice device, const std::vector<char>& code) : m_device{device} { createShaderModule(m_device, code); }
            // ~Shaders() { vkDestroyShaderModule(m_device, m_shaderModule, nullptr); }
            ~Shaders() = default;

            Shaders(const Shaders&) = delete;
            Shaders& operator=(const Shaders&) = delete;
            Shaders(Shaders&&) = delete;
            Shaders& operator=(Shaders&&) = delete;

            void deleteResources() const { vkDestroyShaderModule(m_device, m_shaderModule, nullptr); }

            [[nodiscard]] VkShaderModule getShaderModule() const { return m_shaderModule; }

        private:

            void createShaderModule(const VkDevice& device, const std::vector<char>& code);

            VkDevice m_device = VK_NULL_HANDLE;
            VkShaderModule m_shaderModule = VK_NULL_HANDLE;

    }; // class Shaders

} // namespace ven
