#pragma once

#include <cstring>
#include <memory>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "Utils/Clock.hpp"
#include "Utils/Image.hpp"
#include "Utils/Logger.hpp"
#include "Utils/Utils.hpp"
#include "VEngine/Core/Device.hpp"
#include "VEngine/GFX/Gui.hpp"
#include "VEngine/GFX/Model.hpp"
#include "VEngine/GFX/Renderer.hpp"
#include "VEngine/GFX/Shaders.hpp"

static constexpr uint32_t WIDTH = 1920;
static constexpr uint32_t HEIGHT = 1080;

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
    Engine(): window(WIDTH, HEIGHT), m_device(window), m_gui(m_device, window.getGLFWwindow()) {}

    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:

    VkDescriptorSetLayout descriptorSetLayout = nullptr;
    VkPipelineLayout pipelineLayout = nullptr;

    uint32_t mipLevels = 0;
    VkImage textureImage = nullptr;
    VkDeviceMemory textureImageMemory = nullptr;
    VkImageView textureImageView = nullptr;
    VkSampler textureSampler = nullptr;

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
    ven::Gui m_gui;
    utl::Clock m_clock;

    void initVulkan() {
        createDescriptorSetLayout();
        m_shadersModule.createGraphicsPipeline(m_device.getMsaaSamples(), descriptorSetLayout, pipelineLayout, m_renderer.getSwapChain().getRenderPass());
        createTextureImage(TEXTURE_PATH);
        m_renderer.getSwapChain().createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, textureImageView);
        createTextureSampler();
        loadModel();
        createVertexBuffer();
        createIndexBuffer();
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
        vkDeviceWaitIdle(m_device.getVkDevice());
    }

    void cleanup() {
        m_renderer.getSwapChain().cleanupSwapChain();

        vkDestroyPipeline(m_device.getVkDevice(), m_shadersModule.getGraphicsPipeline(), nullptr);
        vkDestroyPipelineLayout(m_device.getVkDevice(), pipelineLayout, nullptr);
        vkDestroyRenderPass(m_device.getVkDevice(), m_renderer.getSwapChain().getRenderPass(), nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(m_device.getVkDevice(), uniformBuffers[i], nullptr);
            vkFreeMemory(m_device.getVkDevice(), uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(m_device.getVkDevice(), descriptorPool, nullptr);

        vkDestroySampler(m_device.getVkDevice(), textureSampler, nullptr);
        vkDestroyImageView(m_device.getVkDevice(), textureImageView, nullptr);

        vkDestroyImage(m_device.getVkDevice(), textureImage, nullptr);
        vkFreeMemory(m_device.getVkDevice(), textureImageMemory, nullptr);

        vkDestroyDescriptorSetLayout(m_device.getVkDevice(), descriptorSetLayout, nullptr);

        vkDestroyBuffer(m_device.getVkDevice(), indexBuffer, nullptr);
        vkFreeMemory(m_device.getVkDevice(), indexBufferMemory, nullptr);

        vkDestroyBuffer(m_device.getVkDevice(), vertexBuffer, nullptr);
        vkFreeMemory(m_device.getVkDevice(), vertexBufferMemory, nullptr);
    }

    void recreateSwapChain() {
        int width = 0;
        int height = 0;
        window.getFramebufferSize(width, height);
        while (width == 0 || height == 0) {
            window.getFramebufferSize(width, height);
            ven::Window::waitEvents();
        }

        vkDeviceWaitIdle(m_device.getVkDevice());
        m_renderer.getSwapChain().cleanupSwapChain();
        m_renderer.getSwapChain().init();
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

    void createTextureImage(const std::string &filepath) {
        const utl::Image image(filepath);
        const auto imageSize = image.width * image.height * 4;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(image.width, image.height)))) + 1;

        if (image.pixels == nullptr) {
            utl::THROW_ERROR("failed to load texture image");
        }

        VkBuffer stagingBuffer = nullptr;
        VkDeviceMemory stagingBufferMemory = nullptr;
        m_device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data = nullptr;
        vkMapMemory(m_device.getVkDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, image.pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device.getVkDevice(), stagingBufferMemory);

        m_renderer.getSwapChain().createImage(image.width, image.height, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

        m_device.transitionImageLayout(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        m_device.copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(image.width), static_cast<uint32_t>(image.height));
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

        vkDestroyBuffer(m_device.getVkDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_device.getVkDevice(), stagingBufferMemory, nullptr);

        generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, image.width, image.height, mipLevels);
    }

    void generateMipmaps(const VkImage image, VkFormat imageFormat, const int32_t texWidth, const int32_t texHeight, const uint32_t mipLevels) const {
        // Check if image format supports linear blitting
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(m_device.getPhysicalDevice(), imageFormat, &formatProperties);

        if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) == VK_FALSE) {
            utl::THROW_ERROR("texture image format does not support linear blitting!");
        }
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        m_device.beginSingleTimeCommands(commandBuffer);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = {.x=0, .y=0, .z=0};
            blit.srcOffsets[1] = {.x=mipWidth, .y=mipHeight, .z=1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {.x=0, .y=0, .z=0};
            blit.dstOffsets[1] = { .x=mipWidth > 1 ? mipWidth / 2 : 1, .y=mipHeight > 1 ? mipHeight / 2 : 1, .z=1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (mipWidth > 1) { mipWidth /= 2; }
            if (mipHeight > 1) { mipHeight /= 2; }
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        m_device.endSingleTimeCommands(commandBuffer);
    }

    void createTextureSampler() {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_device.getPhysicalDevice(), &properties);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0F;
        samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
        samplerInfo.mipLodBias = 0.0F;

        if (vkCreateSampler(m_device.getVkDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            utl::THROW_ERROR("failed to create texture sampler!");
        }
    }

    void loadModel() {
        const ven::Model model(MODEL_PATH);
        vertices.resize(model.getVertices().size());
        std::ranges::copy(model.getVertices(), vertices.begin());
        indices.resize(model.getIndices().size());
        std::ranges::copy(model.getIndices(), indices.begin());
    }

    void createVertexBuffer() {
        const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer = nullptr;
        VkDeviceMemory stagingBufferMemory = nullptr;
        m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data = nullptr;
        vkMapMemory(m_device.getVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(m_device.getVkDevice(), stagingBufferMemory);

        m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

        m_device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

        vkDestroyBuffer(m_device.getVkDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_device.getVkDevice(), stagingBufferMemory, nullptr);
    }

    void createIndexBuffer() {
        const VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

        VkBuffer stagingBuffer = nullptr;
        VkDeviceMemory stagingBufferMemory = nullptr;
        m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data = nullptr;
        vkMapMemory(m_device.getVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
            memcpy(data, indices.data(), (size_t) bufferSize);
        vkUnmapMemory(m_device.getVkDevice(), stagingBufferMemory);

        m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

        m_device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

        vkDestroyBuffer(m_device.getVkDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_device.getVkDevice(), stagingBufferMemory, nullptr);
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
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

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

            const VkDescriptorImageInfo imageInfo{ .sampler = textureSampler, .imageView = textureImageView, .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
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

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0F, 0.0F, 0.0F, 1.0F}};
        clearValues[1].depthStencil = {.depth=1.0F, .stencil=0};

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
            recreateSwapChain();
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

        const std::array<VkSemaphore, 1> waitSemaphores = {m_renderer.getSwapChain().getImageAvailableSemaphores()[currentFrame]};
        constexpr std::array<VkPipelineStageFlags, 1> waitStages = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores = waitSemaphores.data();
        submitInfo.pWaitDstStageMask = waitStages.data();

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_renderer.getCommandBuffers().at(currentFrame);

        const std::array<VkSemaphore, 1> signalSemaphores = {m_renderer.getSwapChain().getRenderFinishedSemaphores()[currentFrame]};
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
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            utl::THROW_ERROR("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};
