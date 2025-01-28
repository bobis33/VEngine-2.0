///
/// @file Gui.hpp
/// @brief This file contains the Gui class
/// @namespace ven
///

#pragma once

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

#include "VEngine/Core/Device.hpp"

namespace ven {

    ///
    /// @class Gui
    /// @brief Class for Gui
    /// @namespace ven
    ///
    class Gui {

        public:

            Gui(const Device& device, GLFWwindow* window);
            ~Gui() { ImGui_ImplVulkan_Shutdown(); ImGui_ImplGlfw_Shutdown(); ImGui::DestroyContext(); vkDestroyDescriptorPool(m_device, m_pool, nullptr); }

            Gui(const Gui&) = delete;
            Gui& operator=(const Gui&) = delete;
            Gui(Gui&&) = delete;
            Gui& operator=(Gui&&) = delete;

            void render(const VkCommandBuffer& commandBuffer) const;

        private:

            const VkDevice& m_device;
            VkDescriptorPool m_pool = VK_NULL_HANDLE;
            VkPhysicalDeviceProperties m_deviceProperties{};

    }; // class Gui

} // namespace ven
