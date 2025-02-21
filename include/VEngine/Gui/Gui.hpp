///
/// @file Gui.hpp
/// @brief This file contains the Gui class
/// @namespace ven
///

#pragma once

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
                BlackWhite = 0,
                BlueGrey,
                BlackRed,
            };

            Gui(const Device& device, GLFWwindow* window);
            ~Gui();

            Gui(const Gui&) = delete;
            Gui& operator=(const Gui&) = delete;
            Gui(Gui&&) = delete;
            Gui& operator=(Gui&&) = delete;

            void render(const VkCommandBuffer& commandBuffer) const;

            static void applyTheme(const Theme theme) { switch (theme) { case BlackRed: blackRedTheme(); break; case BlackWhite: blackWhiteTheme(); break; case BlueGrey: blueGreyTheme(); } }

        private:

            static void blackRedTheme();
            static void blackWhiteTheme();
            static void blueGreyTheme();

            const VkDevice& m_device;
            VkDescriptorPool m_pool = VK_NULL_HANDLE;
            VkPhysicalDeviceProperties m_deviceProperties{};

    }; // class Gui

} // namespace ven
