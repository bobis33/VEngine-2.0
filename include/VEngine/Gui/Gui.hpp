///
/// @file Gui.hpp
/// @brief This file contains the Gui class
/// @namespace ven
///

#pragma once

#include "Utils/FrameStats.hpp"
#include "Utils/MemoryMonitor.hpp"
#include "VEngine/Gfx/Backend/Device.hpp"

namespace ven {

    ///
    /// @class Gui
    /// @brief Class for Gui
    /// @namespace ven
    ///
    class Gui {

        public:

            enum Theme: uint8_t {
                BlackWhite = 0x00,
                BlueGrey = 0x01,
                BlackRed = 0x02
            };

            Gui(const Device& device, GLFWwindow* window, const VkRenderPass& renderPass);
            ~Gui();

            Gui(const Gui&) = delete;
            Gui& operator=(const Gui&) = delete;
            Gui(Gui&&) = delete;
            Gui& operator=(Gui&&) = delete;

            void render(const VkCommandBuffer& commandBuffer);
            static void applyTheme(const Theme theme) { switch (theme) { case BlackRed: blackRedTheme(); break; case BlackWhite: blackWhiteTheme(); break; case BlueGrey: blueGreyTheme(); } }

        private:

            static void blackRedTheme();
            static void blackWhiteTheme();
            static void blueGreyTheme();

            const VkDevice& m_device;
            VkDescriptorPool m_pool;
            VkPhysicalDeviceProperties m_deviceProperties;
            float m_graphMaxFps{10000.0F};
            FrameStats m_frameStats;
            MemoryMonitor m_memoryMonitor;
    }; // class Gui

} // namespace ven
