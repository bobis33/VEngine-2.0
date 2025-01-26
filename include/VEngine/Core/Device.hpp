///
/// @file Device.hpp
/// @brief This file contains the Device class
/// @namespace ven
///

#pragma once

#include <array>
#include <vector>
#include <optional>

#include "VEngine/Core/Window.hpp"

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

namespace ven {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        [[nodiscard]] bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    ///
    /// @class Device
    /// @brief Class for device
    /// @namespace ven
    ///
    class Device {

        public:

            explicit Device(const Window& window): m_window{window} { createInstance(); setupDebugMessenger(); m_window.createWindowSurface(m_instance, &m_surface); pickPhysicalDevice(); createLogicalDevice(); }
            ~Device();

            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
            void beginSingleTimeCommands(VkCommandBuffer& commandBuffer) const;
            void endSingleTimeCommands(const VkCommandBuffer& commandBuffer) const;
            [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
            [[nodiscard]] QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device) const;
            [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device) const;

            [[nodiscard]] const VkDevice& getVkDevice() const { return m_device; }
            [[nodiscard]] const VkInstance& getVkInstance() const { return m_instance; }
            [[nodiscard]] const VkSurfaceKHR& getVkSurface() const { return m_surface; }
            [[nodiscard]] const VkSampleCountFlagBits& getMsaaSamples() const { return m_msaaSamples; }
            [[nodiscard]] const VkPhysicalDevice& getPhysicalDevice() const { return m_physicalDevice; }
            [[nodiscard]] const VkQueue& getGraphicsQueue() const { return m_graphicsQueue; }
            [[nodiscard]] const VkQueue& getPresentQueue() const { return m_presentQueue; }
            [[nodiscard]] VkCommandPool& getCommandPool() { return m_commandPool; }

        private:

            void createInstance();
            void setupDebugMessenger();
            void pickPhysicalDevice();
            void createLogicalDevice();

            [[nodiscard]] static std::vector<const char *> getRequiredExtensions();
            [[nodiscard]] VkSampleCountFlagBits getMaxUsableSampleCount() const;
            [[nodiscard]] static bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);
            [[nodiscard]] static bool checkValidationLayerSupport();
            [[nodiscard]] bool isDeviceSuitable(const VkPhysicalDevice& device) const;
            static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

            const Window& m_window;

            VkDevice m_device = VK_NULL_HANDLE;
            VkInstance m_instance = VK_NULL_HANDLE;
            VkSurfaceKHR m_surface = VK_NULL_HANDLE;
            VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
            VkQueue m_graphicsQueue = VK_NULL_HANDLE;
            VkQueue m_presentQueue = VK_NULL_HANDLE;
            VkCommandPool m_commandPool = VK_NULL_HANDLE;
            VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
            VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;

            static constexpr std::array<const char*, 1> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
            static constexpr std::array<const char*, 1> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

    }; // class Device

} // namespace ven
