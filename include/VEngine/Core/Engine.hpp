///
/// @file Engine.hpp
/// @brief This file contains the Engine class
/// @namespace ven
///

#pragma once

#include "Utils/Clock.hpp"
#include "VEngine/Core/EventManager.hpp"
#include "VEngine/Gfx/Renderer.hpp"
#include "VEngine/Gfx/Shaders.hpp"
#include "VEngine/Gfx/Backend/Descriptors/Pool.hpp"
#include "VEngine/Gfx/Backend/Descriptors/SetLayout.hpp"

namespace ven {

    ///
    /// @class Engine
    /// @brief Class for engine
    /// @namespace ven
    ///
    class Engine {

        public:

            struct UniformBufferObject {
                alignas(16) glm::mat4 model;
                alignas(16) glm::mat4 view;
                alignas(16) glm::mat4 proj;
            };

            Engine(): m_window(Window::DEFAULT_WIDTH, Window::DEFAULT_HEIGHT), m_device(m_window),
                      m_descriptorPool(m_device.getVkDevice()), m_descriptorSetLayout(m_device.getVkDevice()),
                      m_renderer(m_window, m_device), m_shadersModule(m_device.getVkDevice()),
                      m_eventManager(m_renderer.getCamera(), m_window) { initVulkan(); }

            ~Engine() { cleanup(); }

            Engine(const Engine &) = delete;
            Engine &operator=(const Engine &) = delete;
            Engine(Engine &&) = delete;
            Engine &operator=(Engine &&) = delete;

            void run();

        private:

            void initVulkan();
            void loadAssets();
            void cleanup() const;
            void createUniformBuffers();
            void createDescriptorSets();
            void updateUniformBuffer(uint32_t currentImage);
            void drawFrame();

            VkPipelineLayout pipelineLayout = nullptr;
            VkBuffer vertexBuffer = nullptr;
            VkDeviceMemory vertexBufferMemory = nullptr;
            VkBuffer indexBuffer = nullptr;
            VkDeviceMemory indexBufferMemory = nullptr;
            std::vector<VkBuffer> uniformBuffers;
            std::vector<VkDeviceMemory> uniformBuffersMemory;
            std::vector<void*> uniformBuffersMapped;
            std::vector<VkDescriptorSet> descriptorSets;
            uint32_t currentFrame = 0;
            uint32_t m_indicesSize = 0;
            static constexpr VkDeviceSize m_uniformBufferSize{sizeof(UniformBufferObject)};
            Window m_window;
            Device m_device;
            DescriptorPool m_descriptorPool;
            DescriptorSetLayout m_descriptorSetLayout;
            Renderer m_renderer;
            Shaders m_shadersModule;
            EventManager m_eventManager;
            utl::Clock m_clock;
            std::vector<Model> m_models;

    }; // class Engine

} // namespace ven
