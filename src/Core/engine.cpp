#include <cstring>

#include "Utils/Logger.hpp"
#include "VEngine/Core/Engine.hpp"
#include "VEngine/Gfx/Resources/TextureManager.hpp"

static const std::string MODEL_PATH = "assets/models/viking_room.obj";
static const std::string MODEL_PATH_2 = "assets/models/flat_vase.obj";
static const std::string MODEL_PATH_3 = "assets/models/sponza/sponza.obj";
static const std::string MODEL_PATH_4 = "assets/models/book.obj";

void ven::Engine::initVulkan() {
    loadAssets();
    m_shadersModule.createGraphicsPipeline(m_device.getMsaaSamples(), m_descriptorSetLayout.getDescriptorSetLayout(), pipelineLayout, m_renderer.getSwapChain().getRenderPass());
    createUniformBuffers();
    createDescriptorSets();
    m_renderer.createCommandBuffers();
}

void ven::Engine::loadAssets() {
    std::vector modelPaths = {MODEL_PATH, MODEL_PATH_3};
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t vertexOffset = 0;
    for (const auto& path : modelPaths) {
        m_models.emplace_back(m_device, m_renderer.getSwapChain(), path);
        Model& model = m_models.back();
        utl::Logger::logInfo("Model at " + path + " has " + std::to_string(model.getMeshes().size()) + " meshes");
        for (const auto& mesh : model.getMeshes()) {
            for (const auto& vertex : mesh->getVertices()) {
                vertices.push_back(vertex);
            }
            for (const auto& index : mesh->getIndices()) {
                indices.push_back(index + vertexOffset);
            }
            vertexOffset += static_cast<uint32_t>(mesh->getVertices().size());
        }
    }
    m_indicesSize = static_cast<uint32_t>(indices.size());
    Model::createBuffer(m_device, vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
    Model::createBuffer(m_device, indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);
}

void ven::Engine::run() {
    while (!m_window.shouldClose()) {
        Window::pollEvents();
        drawFrame();
    }
    m_device.waitIdle();
}

void ven::Engine::cleanup() const {
    const VkDevice& device = m_device.getVkDevice();
    TextureManager::clean();
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
}

void ven::Engine::createUniformBuffers() {
    uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        m_device.createBuffer(m_uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(m_device.getVkDevice(), uniformBuffersMemory[i], 0, m_uniformBufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void ven::Engine::updateUniformBuffer(const uint32_t currentImage) const {
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0F), m_clock.getDeltaSeconds() * glm::radians(10.0F), glm::vec3(0.0F, 0.0F, 1.0F));
    ubo.view = glm::lookAt(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, 0.0F, 1.0F));
    ubo.proj = glm::perspective(glm::radians(45.0F), static_cast<float>(m_renderer.getSwapChain().getExtent().width) / static_cast<float>(m_renderer.getSwapChain().getExtent().height), 0.1F, 10.0F);
    ubo.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void ven::Engine::createDescriptorSets() {
    const std::vector layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, m_descriptorSetLayout.getDescriptorSetLayout());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool.getDescriptorPool();
    allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();
    descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_device.getVkDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw utl::THROW_ERROR("failed to allocate descriptor sets!");
    }
    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = m_uniformBufferSize;
        std::vector<VkWriteDescriptorSet> descriptorWrites;
        VkWriteDescriptorSet uboWrite{};
        uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        uboWrite.dstSet = descriptorSets[i];
        uboWrite.dstBinding = 0;
        uboWrite.dstArrayElement = 0;
        uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboWrite.descriptorCount = 1;
        uboWrite.pBufferInfo = &bufferInfo;
        descriptorWrites.push_back(uboWrite);
        std::vector<VkDescriptorImageInfo> imageInfos;
        for (const auto& model : m_models) {
            for (const auto& mesh : model.getMeshes()) {
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = mesh->getTexture()->getTextureImageView();
                imageInfo.sampler = mesh->getTexture()->getTextureSampler();
                imageInfos.push_back(imageInfo);
            }
        }
        VkWriteDescriptorSet samplerWrite{};
        samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        samplerWrite.dstSet = descriptorSets[i];
        samplerWrite.dstBinding = 1;
        samplerWrite.dstArrayElement = 0;
        samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerWrite.descriptorCount = 1;
        samplerWrite.pImageInfo = imageInfos.data();
        descriptorWrites.push_back(samplerWrite);
        vkUpdateDescriptorSets(m_device.getVkDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void ven::Engine::drawFrame() {
    uint32_t imageIndex = 0;
    if (vkWaitForFences(m_device.getVkDevice(), 1, &m_renderer.getSwapChain().getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
        throw utl::THROW_ERROR("failed to wait for fence!");
    }
    VkResult result = m_renderer.getSwapChain().acquireNextImage(imageIndex, currentFrame);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_renderer.recreateSwapChain();
        return;
    } if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw utl::THROW_ERROR("failed to acquire swap chain image!");
    }
    updateUniformBuffer(currentFrame);
    vkResetFences(m_device.getVkDevice(), 1, &m_renderer.getSwapChain().getInFlightFences()[currentFrame]);
    vkResetCommandBuffer(m_renderer.getCommandBuffers()[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    m_renderer.recordCommandBuffer(m_renderer.getCommandBuffers()[currentFrame], imageIndex, m_shadersModule.getGraphicsPipeline(), vertexBuffer, indexBuffer, pipelineLayout, m_indicesSize, &descriptorSets[currentFrame]);
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
        throw utl::THROW_ERROR("failed to submit draw command buffer!");
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
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized()) {
        m_window.resetWindowResizedFlag();
        m_renderer.recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw utl::THROW_ERROR("failed to present swap chain image!");
    }
    currentFrame = (currentFrame + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}
