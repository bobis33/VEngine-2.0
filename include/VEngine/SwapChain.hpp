///
/// @file SwapChain.hpp
/// @brief This file contains the SwapChain class
/// @namespace ven
///

#pragma once

#include <vulkan/vulkan_core.h>

namespace ven {

    ///
    /// @class SwapChain
    /// @brief Class for swap chain
    /// @namespace ven
    ///
    class SwapChain {

        public:

            SwapChain() = default;
            ~SwapChain() = default;

            static void createImageView(const VkDevice& device, const VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageView& imageView);

    }; // class SwapChain

} // namespace ven
