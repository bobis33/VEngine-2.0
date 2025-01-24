#pragma once

#include <array>
#include <vector>

#include "VEngine/Window.hpp"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

namespace ven {

    class Device {

        public:

            ~Device() = default;

            static constexpr std::array<const char*, 1> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
            static constexpr std::array<const char*, 1> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };

            void create(VkDevice* device, VkPhysicalDevice* physicalDevice, VkCommandPool* commandPool, VkQueue* graphicsQueue, Window* window) { m_device = device; m_physicalDevice = physicalDevice; m_commandPool = commandPool; m_graphicsQueue = graphicsQueue; m_window = window; }

            void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
            [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
            void beginSingleTimeCommands(VkCommandBuffer& commandBuffer) const;
            void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

            [[nodiscard]] VkDevice* device() const { return m_device; }

            [[nodiscard]] bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
            [[nodiscard]] bool checkValidationLayerSupport() const;
            [[nodiscard]] std::vector<const char *> getRequiredExtensions() const;

        private:

            VkDevice* m_device = VK_NULL_HANDLE;
            VkPhysicalDevice* m_physicalDevice = VK_NULL_HANDLE;
            VkCommandPool* m_commandPool = VK_NULL_HANDLE;
            VkQueue* m_graphicsQueue = VK_NULL_HANDLE;
            Window* m_window = nullptr;

    }; // class Device

} // namespace ven