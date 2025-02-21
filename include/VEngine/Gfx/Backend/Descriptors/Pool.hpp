///
/// @file Pool.hpp
/// @brief This file contains the DescriptorPool class
/// @namespace ven
///

#pragma once

#include "VEngine/Gfx/Backend/SwapChain.hpp"

namespace ven {

    ///
    /// @class DescriptorPool
    /// @brief Class for Descriptorpool
    /// @namespace ven
    ///
    class DescriptorPool {

        public:
        explicit DescriptorPool(const VkDevice& device) : m_device{device} {
                static constexpr std::array<VkDescriptorPoolSize, 2> poolSizes {{
                { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT) },
                { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT) }
            }};
                static constexpr VkDescriptorPoolCreateInfo poolInfo {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    .maxSets = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT),
                    .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                    .pPoolSizes = poolSizes.data()
                };
                if (vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
                    throw utl::THROW_ERROR("failed to create descriptor pool!");
                }
            }
            ~DescriptorPool() { vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr); }

            DescriptorPool(const DescriptorPool &) = delete;
            DescriptorPool &operator=(const DescriptorPool &) = delete;
            DescriptorPool(DescriptorPool &&) = delete;
            DescriptorPool &operator=(DescriptorPool &&) = delete;

            [[nodiscard]] const VkDescriptorPool& getDescriptorPool() const { return m_descriptorPool; }

        private:

            const VkDevice& m_device;
            VkDescriptorPool m_descriptorPool = nullptr;

    }; // class DescriptorPool

} // namespace ven
