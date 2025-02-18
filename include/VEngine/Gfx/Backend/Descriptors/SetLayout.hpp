///
/// @file SetLayout.hpp
/// @brief This file contains the DescriptorSetLayout class
/// @namespace ven
///

#pragma once

 #include "VEngine/Gfx/Backend/Device.hpp"

namespace ven {

    ///
    /// @class DescriptorSetLayout
    /// @brief Class for DescriptorSetlayout
    /// @namespace ven
    ///
    class DescriptorSetLayout {

        public:

            explicit DescriptorSetLayout(const VkDevice &device): m_device{device} {
                VkDescriptorSetLayoutBinding uboLayoutBinding{};
                uboLayoutBinding.binding = 0;
                uboLayoutBinding.descriptorCount = 1;
                uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                uboLayoutBinding.pImmutableSamplers = nullptr;
                uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                VkDescriptorSetLayoutBinding samplerLayoutBinding{};
                samplerLayoutBinding.binding = 1;
                samplerLayoutBinding.descriptorCount = 1;
                samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                samplerLayoutBinding.pImmutableSamplers = nullptr;
                samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                const std::array bindings = {uboLayoutBinding, samplerLayoutBinding};
                VkDescriptorSetLayoutCreateInfo layoutInfo{};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
                layoutInfo.pBindings = bindings.data();
                if (vkCreateDescriptorSetLayout(m_device, &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS) {
                    throw utl::THROW_ERROR("failed to create descriptor set layout!");
                }
            }
            ~DescriptorSetLayout() { vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr); }

            DescriptorSetLayout(const DescriptorSetLayout &) = delete;
            DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;
            DescriptorSetLayout(DescriptorSetLayout &&) = delete;
            DescriptorSetLayout &operator=(DescriptorSetLayout &&) = delete;

            [[nodiscard]] const VkDescriptorSetLayout& getDescriptorSetLayout() const { return m_descriptorSetLayout; }

        private:

            const VkDevice& m_device;
            VkDescriptorSetLayout m_descriptorSetLayout = nullptr;

    }; // class DescriptorSetLayout

} // namespace ven
