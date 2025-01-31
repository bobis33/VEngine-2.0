#pragma once

#include <memory>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "Utils/Clock.hpp"
#include "VEngine/Gfx/Gui.hpp"
#include "VEngine/Gfx/Model.hpp"
#include "VEngine/Gfx/Texture.hpp"
#include "VEngine/Gfx/Renderer.hpp"
#include "VEngine/Gfx/Shaders.hpp"

static const std::string MODEL_PATH = "assets/models/viking_room.obj";
static const std::string MODEL_PATH_2 = "assets/models/flat_vase.obj";
static const std::string TEXTURE_PATH = "assets/textures/viking_room.png";
static const std::string TEXTURE_PATH_2 = "assets/textures/default.png";

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct RenderObject {
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    uint32_t indexCount;
    uint32_t textureIndex; // L'indice de la texture associée à cet objet
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

namespace ven {

    class Engine {

        public:

            Engine(): m_window(Window::DEFAULT_WIDTH, Window::DEFAULT_HEIGHT), m_device(m_window), m_renderer(m_window, m_device), m_gui(m_device, m_window.getGLFWwindow()) { initVulkan(); }
            ~Engine() { cleanup(); }

            void run() { mainLoop(); }

        private:

            VkDescriptorSetLayout descriptorSetLayout = nullptr;
            VkPipelineLayout pipelineLayout = nullptr;
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            VkBuffer vertexBuffer = nullptr;
            VkDeviceMemory vertexBufferMemory = nullptr;
            VkBuffer indexBuffer = nullptr;
            VkDeviceMemory indexBufferMemory = nullptr;
            std::vector<VkBuffer> uniformBuffers;
            std::vector<VkDeviceMemory> uniformBuffersMemory;
            std::vector<void*> uniformBuffersMapped;
            VkDescriptorPool descriptorPool = nullptr;
            std::vector<VkDescriptorSet> descriptorSets;
            uint32_t currentFrame = 0;
            Window m_window;
            Device m_device;
            Renderer m_renderer;
            Shaders m_shadersModule{m_device.getVkDevice()};
            Gui m_gui;
            utl::Clock m_clock;
            std::vector<std::unique_ptr<Texture>> m_textures;
            std::vector<RenderObject> renderObjects;

            void initVulkan();
            void loadAssets();
            void mainLoop();
            void cleanup() const;
            void createDescriptorSetLayout();
            void createUniformBuffers();
            void createDescriptorPool();
            void createDescriptorSets();
            void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
            void updateUniformBuffer(uint32_t currentImage);
            void drawFrame();
    };
}
