///
/// @file Model.hpp
/// @brief This file contains the model class
/// @namespace ven
///

#pragma once

#include "VEngine/Vertex.hpp"
#include "VEngine/Core/Device.hpp"

namespace ven {

    ///
    /// @class Model
    /// @brief Class for models
    /// @namespace ven
    ///
    class Model {

        public:

            explicit Model(const Device& device, const std::string &path);
            ~Model() = default;

            Model(const Model&) = delete;
            Model& operator=(const Model&) = delete;
            Model(Model&&) = delete;
            Model& operator=(Model&&) = delete;

            template<typename T>
            static void createBuffer(const Device& device, const std::vector<T>& data, const VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
                void* mappedData = nullptr;
                VkBuffer stagingBuffer = nullptr;
                VkDeviceMemory stagingBufferMemory = nullptr;
                const VkDeviceSize bufferSize = sizeof(data[0]) * data.size();
                const VkDevice& vkDevice = device.getVkDevice();
                device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
                vkMapMemory(vkDevice, stagingBufferMemory, 0, bufferSize, 0, &mappedData);
                memcpy(mappedData, data.data(), static_cast<size_t>(bufferSize));
                vkUnmapMemory(vkDevice, stagingBufferMemory);
                device.createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
                device.copyBuffer(stagingBuffer, buffer, bufferSize);
                vkDestroyBuffer(vkDevice, stagingBuffer, nullptr);
                vkFreeMemory(vkDevice, stagingBufferMemory, nullptr);
            }

            [[nodiscard]] const std::vector<Vertex>& getVertices() const { return m_vertices; }
            [[nodiscard]] const std::vector<uint32_t>& getIndices() const { return m_indices; }

        private:

            const Device& m_device;

            std::vector<Vertex> m_vertices;
            std::vector<uint32_t> m_indices;

    }; // class Model

} // namespace ven
