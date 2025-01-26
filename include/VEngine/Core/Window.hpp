///
/// @file Window.hpp
/// @brief This file contains the Window class
/// @namespace ven
///

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Utils/ErrorHandling.hpp"

namespace ven {

    static constexpr std::string_view DEFAULT_TITLE = "VEngine";
    static constexpr uint32_t DEFAULT_WIDTH = 1920;
    static constexpr uint32_t DEFAULT_HEIGHT = 1080;

    ///
    /// @class Window
    /// @brief Class for window
    /// @namespace ven
    ///
    class Window {

        public:

            explicit Window(const uint32_t width = DEFAULT_WIDTH, const uint32_t height = DEFAULT_HEIGHT) : m_window(createWindow(width, height, DEFAULT_TITLE.data())) { setWindowIcon("assets/icons/icon64x64.png"); }
            ~Window() { glfwDestroyWindow(m_window); glfwTerminate(); m_window = nullptr; }

            Window(const Window&) = delete;
            Window& operator=(const Window&) = delete;
            Window(Window&&) = delete;
            Window& operator=(Window&&) = delete;

            void createWindowSurface(const VkInstance& instance, VkSurfaceKHR* surface) const { if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) { THROW_ERROR("Failed to create window surface"); } }
            static void setFullscreen(bool fullscreen, uint32_t width, uint32_t height);
            [[nodiscard]] bool wasWindowResized() const { return m_framebufferResized; }
            void resetWindowResizedFlag() { m_framebufferResized = false; }
            static void pollEvents() { glfwPollEvents(); }
            [[nodiscard]] bool shouldClose() const { return glfwWindowShouldClose(m_window) != 0; }
            static void waitEvents() { glfwWaitEvents(); }

            [[nodiscard]] VkExtent2D getExtent() const { int width = 0; int height = 0; glfwGetFramebufferSize(m_window, &width, &height); return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
            void getFramebufferSize(int& width, int& height) const { glfwGetFramebufferSize(m_window, &width, &height); }
            [[nodiscard]] static const char **getRequiredInstanceExtensions(uint32_t *count) { return glfwGetRequiredInstanceExtensions(count); }
            [[nodiscard]] GLFWwindow* getGLFWwindow() const { return m_window; }

        private:

            [[nodiscard]] GLFWwindow* createWindow(uint32_t width, uint32_t height, const std::string &title);
            void setWindowIcon(const std::string& path) const;
            static void framebufferResizeCallback(GLFWwindow* window, int width, int height) { static_cast<Window *>(glfwGetWindowUserPointer(window))->m_framebufferResized = true; }

            GLFWwindow* m_window = nullptr;
            bool m_framebufferResized = false;

    }; // class Window

} // namespace ven
