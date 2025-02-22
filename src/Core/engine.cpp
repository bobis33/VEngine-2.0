#include "Utils/Logger.hpp"
#include "VEngine/Core/Engine.hpp"
#include "VEngine/Gfx/Resources/TextureManager.hpp"

void ven::Engine::initVulkan() {
    loadAssets();
    m_descriptorSetLayout.create(TextureManager::getTextureSize());
    m_shadersModule.createGraphicsPipeline(m_device.getMsaaSamples(), m_descriptorSetLayout.getDescriptorSetLayout(), pipelineLayout, m_renderer.getSwapChain().getRenderPass());
    createUniformBuffers();
    createDescriptorSets();
    m_renderer.createCommandBuffers();
}

void ven::Engine::loadAssets() {
    std::vector<std::string> modelPaths = {"assets/models/sponza/sponza.obj"};
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t vertexOffset = 0;
    for (const auto& path : modelPaths) {
        utl::Logger::logExecutionTime("Loading model: " + path, [&] { m_models.emplace_back(m_device, m_renderer.getSwapChain(), path); });
        for (Model& model = m_models.back(); const auto& mesh : model.getMeshes()) {
            for (const auto& vertex : mesh->getVertices()) {
                vertices.push_back(vertex);
            }
            for (const auto& index : mesh->getIndices()) {
                indices.push_back(index + vertexOffset);
            }
            vertexOffset += static_cast<uint32_t>(mesh->getVertices().size());
        }
    }
    utl::Logger::logInfo("Textures loaded: " + std::to_string(TextureManager::getTextureSize()));
    m_indicesSize = static_cast<uint32_t>(indices.size());
    Model::createBuffer(m_device, vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
    Model::createBuffer(m_device, indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);
}

void ven::Engine::run() {
    while (!m_window.shouldClose()) {
        m_eventManager.handleEvents(m_clock.getDeltaSeconds());
        m_clock.restart();
        drawFrame();
    }
    m_device.waitIdle();
}

void ven::Engine::cleanup() const {
    const VkDevice& device = m_device.getVkDevice();
    TextureManager::clean();
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    for (uint8_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
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
    for (uint8_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
        m_device.createBuffer(m_uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(m_device.getVkDevice(), uniformBuffersMemory[i], 0, m_uniformBufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void ven::Engine::updateUniformBuffer(const uint32_t currentImage) const {
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0F);
    ubo.view = m_camera.getViewMatrix();
    ubo.proj = m_camera.getProjectionMatrix(static_cast<float>(m_renderer.getSwapChain().getExtent().width) / static_cast<float>(m_renderer.getSwapChain().getExtent().height));
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
    for (uint8_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
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
        for (uint8_t textureIndex = 0; textureIndex < TextureManager::getTextureSize(); textureIndex++) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = TextureManager::getTexturePath(textureIndex)->getTextureImageView();
            imageInfo.sampler = TextureManager::getTexturePath(textureIndex)->getTextureSampler();
            imageInfos.push_back(imageInfo);
        }
        VkWriteDescriptorSet samplerWrite{};
        samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        samplerWrite.dstSet = descriptorSets[i];
        samplerWrite.dstBinding = 1;
        samplerWrite.dstArrayElement = 0;
        samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());
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
