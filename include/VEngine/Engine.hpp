#pragma once

#include <memory>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "Utils/Logger.hpp"
#include "VEngine/Gfx/Gui.hpp"
#include "VEngine/Gfx/Model.hpp"
#include "VEngine/Gfx/Texture.hpp"
#include "VEngine/Gfx/Renderer.hpp"
#include "VEngine/Gfx/Shaders.hpp"

static const std::string MODEL_PATH = "assets/models/viking_room.obj";
static const std::string TEXTURE_PATH = "assets/textures/viking_room.png";

static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Engine {
public:
    Engine(): window(ven::Window::DEFAULT_WIDTH, ven::Window::DEFAULT_HEIGHT), m_device(window), m_gui(m_device, window.getGLFWwindow()) {}
    ~Engine() { cleanup(); }

    void run() {
        initVulkan();
        mainLoop();
    }

private:

    VkDescriptorSetLayout descriptorSetLayout = nullptr;
    VkPipelineLayout pipelineLayout = nullptr;

    std::vector<ven::Vertex> vertices;
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

    ven::Window window;
    ven::Device m_device;
    ven::Renderer m_renderer{window, m_device};
    ven::Shaders m_shadersModule{m_device.getVkDevice()};
    ven::Texture m_texture{m_device};
    ven::Gui m_gui;
    utl::Clock m_clock;

    void initVulkan() {
        createDescriptorSetLayout();
        m_shadersModule.createGraphicsPipeline(m_device.getMsaaSamples(), descriptorSetLayout, pipelineLayout, m_renderer.getSwapChain().getRenderPass());
        m_texture.createTextureImage(TEXTURE_PATH, m_renderer.getSwapChain());
        m_renderer.getSwapChain().createImageView(m_texture.getTextureImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_texture.getMipLevels(), m_texture.getTextureImageView());
        m_texture.createTextureSampler();
        loadModel();
        createBuffer(vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
        createBuffer(indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        m_renderer.createCommandBuffers();
    }

    void mainLoop() {
        while (!window.shouldClose()) {
            ven::Window::pollEvents();
            utl::Logger::logExecutionTime("drawFrame", [&] { drawFrame(); });
        }
        m_device.waitIdle();
    }

    void cleanup() const {
        const VkDevice& device = m_device.getVkDevice();
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, uniformBuffers[i], nullptr);
            vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
        }
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        vkDestroyBuffer(device, indexBuffer, nullptr);
        vkFreeMemory(device, indexBufferMemory, nullptr);
        vkDestroyBuffer(device, vertexBuffer, nullptr);
        vkFreeMemory(device, vertexBufferMemory, nullptr);
    }

    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        const std::array bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(m_device.getVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            utl::THROW_ERROR("failed to create descriptor set layout!");
        }
    }

    void loadModel() {
        const ven::Model model(MODEL_PATH);
        vertices.resize(model.getVertices().size());
        std::ranges::copy(model.getVertices(), vertices.begin());
        indices.resize(model.getIndices().size());
        std::ranges::copy(model.getIndices(), indices.begin());
    }

    template<typename T>
    void createBuffer(const std::vector<T>& data, const VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        void* mappedData = nullptr;
        VkBuffer stagingBuffer = nullptr;
        VkDeviceMemory stagingBufferMemory = nullptr;
        const VkDeviceSize bufferSize = sizeof(data[0]) * data.size();
        const VkDevice& device = m_device.getVkDevice();
        m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &mappedData);
        memcpy(mappedData, data.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(device, stagingBufferMemory);
        m_device.createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
        m_device.copyBuffer(stagingBuffer, buffer, bufferSize);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void createUniformBuffers() {
        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);
            m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
            vkMapMemory(m_device.getVkDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }
    }

    void createDescriptorPool() {
        static constexpr std::array<VkDescriptorPoolSize, 2> poolSizes {{
            { .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) },
            { .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) }
        }};
        static constexpr VkDescriptorPoolCreateInfo poolInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data()
        };
        if (vkCreateDescriptorPool(m_device.getVkDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            utl::THROW_ERROR("failed to create descriptor pool!");
        }
    }

    void createDescriptorSets() {
        const std::vector layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();
        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(m_device.getVkDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            utl::THROW_ERROR("failed to allocate descriptor sets!");
        }
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);
            const VkDescriptorImageInfo imageInfo{ .sampler = m_texture.getTextureSampler(), .imageView = m_texture.getTextureImageView(), .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
            vkUpdateDescriptorSets(m_device.getVkDevice(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        }
    }

    void recordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            utl::THROW_ERROR("failed to begin recording command buffer!");
        }
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderer.getSwapChain().getRenderPass();
        renderPassInfo.framebuffer = m_renderer.getSwapChain().getSwapChainFrameBuffers()[imageIndex];
        renderPassInfo.renderArea.offset = {.x=0, .y=0};
        renderPassInfo.renderArea.extent = m_renderer.getSwapChain().getExtent();
        static constexpr std::array clearValues = {VkClearValue{.color = {0.0F, 0.0F, 0.0F, 1.0F}}, VkClearValue{.depthStencil = {1.0F, 0}}};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadersModule.getGraphicsPipeline());
        const VkViewport viewport{ .x=0.0F, .y=0.0F, .width=static_cast<float>(m_renderer.getSwapChain().getExtent().width), .height=static_cast<float>(m_renderer.getSwapChain().getExtent().height), .minDepth=0.0f, .maxDepth=1.0f };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        const VkRect2D scissor{ .offset = {.x=0, .y=0}, .extent = m_renderer.getSwapChain().getExtent() };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        const std::array vertexBuffers = {vertexBuffer};
        constexpr std::array<VkDeviceSize, 1> offsets = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers.data(), offsets.data());
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        m_gui.render(commandBuffer);
        vkCmdEndRenderPass(commandBuffer);
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            utl::THROW_ERROR("failed to record command buffer!");
        }
    }

    void updateUniformBuffer(const uint32_t currentImage) {
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), m_clock.getDeltaSeconds() * glm::radians(10.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(m_renderer.getSwapChain().getExtent().width) / static_cast<float>(m_renderer.getSwapChain().getExtent().height), 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
        memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void drawFrame() {
        uint32_t imageIndex = 0;
        vkWaitForFences(m_device.getVkDevice(), 1, &m_renderer.getSwapChain().getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX);
        VkResult result = m_renderer.getSwapChain().acquireNextImage(imageIndex, currentFrame);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            m_renderer.recreateSwapChain();
            return;
        } if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            utl::THROW_ERROR("failed to acquire swap chain image!");
        }
        updateUniformBuffer(currentFrame);
        vkResetFences(m_device.getVkDevice(), 1, &m_renderer.getSwapChain().getInFlightFences()[currentFrame]);
        vkResetCommandBuffer(m_renderer.getCommandBuffers()[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        recordCommandBuffer(m_renderer.getCommandBuffers()[currentFrame], imageIndex);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        const std::array waitSemaphores = {m_renderer.getSwapChain().getImageAvailableSemaphores()[currentFrame]};
        constexpr std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_renderer.getCommandBuffers().at(currentFrame);
        const std::array signalSemaphores = {m_renderer.getSwapChain().getRenderFinishedSemaphores()[currentFrame]};
        submitInfo.signalSemaphoreCount = signalSemaphores.size();
        submitInfo.pSignalSemaphores = signalSemaphores.data();
        if (vkQueueSubmit(m_device.getGraphicsQueue(), 1, &submitInfo, m_renderer.getSwapChain().getInFlightFences()[currentFrame]) != VK_SUCCESS) {
            utl::THROW_ERROR("failed to submit draw command buffer!");
        }
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = signalSemaphores.size();
        presentInfo.pWaitSemaphores = signalSemaphores.data();
        const std::array swapChains = {m_renderer.getSwapChain().getSwapChain()};
        presentInfo.swapchainCount = swapChains.size();
        presentInfo.pSwapchains = swapChains.data();
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(m_device.getPresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
            window.resetWindowResizedFlag();
            m_renderer.recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            utl::THROW_ERROR("failed to present swap chain image!");
        }
        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};
