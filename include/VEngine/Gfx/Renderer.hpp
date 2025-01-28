///
/// @file Renderer.hpp
/// @brief This file contains the Renderer class
/// @namespace ven
///

#pragma once

#include "VEngine/Gfx/SwapChain.hpp"

namespace ven {

    ///
    /// @class Renderer
    /// @brief Class for renderer
    /// @namespace ven
    ///
    class Renderer {

        public:

            Renderer(const Window &window, const Device &device) : m_device{device}, m_window{window}, m_swapChain{m_device, window.getExtent()} {  }
            ~Renderer() = default;

            Renderer(const Renderer &) = delete;
            Renderer& operator=(const Renderer &) = delete;
            Renderer(Renderer &&) = delete;
            Renderer& operator=(Renderer &&) = delete;

            void createCommandBuffers();
            void recreateSwapChain();

            [[nodiscard]] SwapChain& getSwapChain() { return m_swapChain; }
            [[nodiscard]] std::vector<VkCommandBuffer>& getCommandBuffers() { return m_commandBuffers; }

        private:

            const Device& m_device;
            const Window& m_window;
            SwapChain m_swapChain;
            std::vector<VkCommandBuffer> m_commandBuffers;

    }; // class Renderer

} // namespace ven
