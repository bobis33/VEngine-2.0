///
/// @file Renderer.hpp
/// @brief This file contains the Renderer class
/// @namespace ven
///

#pragma once

#include "VEngine/Gfx/Resources/Model.hpp"
#include "VEngine/Gui/Gui.hpp"

namespace ven {

    ///
    /// @class Renderer
    /// @brief Class for renderer
    /// @namespace ven
    ///
    class Renderer {

        public:

            Renderer(const Window &window, const Device &device) : m_device(device), m_window(window), m_swapChain(m_device, window.getExtent()), m_gui(m_device, m_camera, m_window.getGLFWWindow(), m_swapChain.getRenderPass(), m_clearValues) {  }
            ~Renderer() = default;

            Renderer(const Renderer &) = delete;
            Renderer& operator=(const Renderer &) = delete;
            Renderer(Renderer &&) = delete;
            Renderer& operator=(Renderer &&) = delete;

            void createCommandBuffers();
            void recreateSwapChain();
            void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, const VkPipeline& graphicsPipeline, const VkBuffer& vertexBuffer, const VkBuffer& indexBuffer, const VkPipelineLayout& pipelineLayout, uint32_t indiceSize, const VkDescriptorSet* descriptorSets);

            [[nodiscard]] const SwapChain& getSwapChain() const { return m_swapChain; }
            [[nodiscard]] const std::vector<VkCommandBuffer>& getCommandBuffers() const { return m_commandBuffers; }
            [[nodiscard]] Camera& getCamera() { return m_camera; }

        private:

            std::array<VkClearValue, 2> m_clearValues{VkClearValue{.color = {0.0F, 0.0F, 0.0F, 1.0F}}, VkClearValue{.depthStencil = {1.0F, 0}}};
            const Device& m_device;
            const Window& m_window;
            SwapChain m_swapChain;
            Gui m_gui;
            Camera m_camera;
            std::vector<VkCommandBuffer> m_commandBuffers;

    }; // class Renderer

} // namespace ven
