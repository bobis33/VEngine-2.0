#include <cstring>

#include "Utils/Logger.hpp"
#include "VEngine/Engine.hpp"

void ven::Engine::initVulkan() {
    loadAssets();
    m_textures.emplace_back(std::make_unique<Texture>(m_device, m_renderer.getSwapChain(), TEXTURE_PATH));
    m_textures.emplace_back(std::make_unique<Texture>(m_device, m_renderer.getSwapChain(), TEXTURE_PATH_2));
    createDescriptorSetLayout();
    m_shadersModule.createGraphicsPipeline(m_device.getMsaaSamples(), descriptorSetLayout, pipelineLayout, m_renderer.getSwapChain().getRenderPass());
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    m_renderer.createCommandBuffers();
}

void ven::Engine::loadAssets() {
    const Model model(m_device, MODEL_PATH);
    const auto vertexOffset = static_cast<uint32_t>(vertices.size());
    vertices.insert(vertices.end(), model.getVertices().begin(), model.getVertices().end());
    for (const uint32_t index : model.getIndices()) {
        indices.push_back(index + vertexOffset);
    }
    Model::createBuffer(m_device, vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory);
    Model::createBuffer(m_device, indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory);
    RenderObject renderObject{};
    renderObject.textureIndex = 0;
    renderObject.vertexBuffer = vertexBuffer;
    renderObject.indexBuffer = indexBuffer;
    renderObject.indexCount = static_cast<uint32_t>(indices.size());
    renderObjects.push_back(renderObject);
}

void ven::Engine::mainLoop() {
    while (!m_window.shouldClose()) {
        Window::pollEvents();
        drawFrame();
    }
    m_device.waitIdle();
}

void ven::Engine::cleanup() const {
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

void ven::Engine::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = static_cast<uint32_t>(m_textures.size());
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

void ven::Engine::createUniformBuffers() {
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        constexpr VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        m_device.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(m_device.getVkDevice(), uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void ven::Engine::createDescriptorPool() {
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

void ven::Engine::createDescriptorSets() {
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
        for (const auto& texture : m_textures) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture->getTextureImageView();
            imageInfo.sampler = texture->getTextureSampler();
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
        vkUpdateDescriptorSets(
        m_device.getVkDevice(),
        static_cast<uint32_t>(descriptorWrites.size()),
        descriptorWrites.data(),
        0,
        nullptr
        );
    }
}

void ven::Engine::recordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        utl::THROW_ERROR("failed to begin recording command buffer!");
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderer.getSwapChain().getRenderPass();
    renderPassInfo.framebuffer = m_renderer.getSwapChain().getSwapChainFrameBuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {.x = 0, .y = 0};
    renderPassInfo.renderArea.extent = m_renderer.getSwapChain().getExtent();
    static constexpr std::array clearValues = {
        VkClearValue{.color = {0.0F, 0.0F, 0.0F, 1.0F}},
        VkClearValue{.depthStencil = {1.0F, 0}}
    };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadersModule.getGraphicsPipeline());
    const VkViewport viewport{
        .x = 0.0F,
        .y = 0.0F,
        .width = static_cast<float>(m_renderer.getSwapChain().getExtent().width),
        .height = static_cast<float>(m_renderer.getSwapChain().getExtent().height),
        .minDepth = 0.0F,
        .maxDepth = 1.0F
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    const VkRect2D scissor{
        .offset = {.x = 0, .y = 0},
        .extent = m_renderer.getSwapChain().getExtent()
    };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    if (vertexBuffer != VK_NULL_HANDLE && !vertices.empty() && !indices.empty()) {
        for (const auto&[vertexBuffer, indexBuffer, indexCount, textureIndex] : renderObjects) {
            PushConstantData pushConstantData{};
            pushConstantData.textureIndex = textureIndex;
            vkCmdPushConstants(
            commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstantData),
            &pushConstantData
            );
            const std::array vertexBuffers = {vertexBuffer};
            constexpr std::array<VkDeviceSize, 1> offsets = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers.data(), offsets.data());
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
    } else {
        utl::Logger::logWarning("No vertices or indices loaded. Skipping draw command.");
    }
    m_gui.render(commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        utl::THROW_ERROR("failed to record command buffer!");
    }
}

void ven::Engine::updateUniformBuffer(const uint32_t currentImage) {
    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0F), m_clock.getDeltaSeconds() * glm::radians(10.0F), glm::vec3(0.0F, 0.0F, 1.0F));
    ubo.view = glm::lookAt(glm::vec3(2.0F, 2.0F, 2.0F), glm::vec3(0.0F, 0.0F, 0.0F), glm::vec3(0.0F, 0.0F, 1.0F));
    ubo.proj = glm::perspective(glm::radians(45.0F), static_cast<float>(m_renderer.getSwapChain().getExtent().width) / static_cast<float>(m_renderer.getSwapChain().getExtent().height), 0.1F, 10.0F);
    ubo.proj[1][1] *= -1;
    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void ven::Engine::drawFrame() {
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
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized()) {
        m_window.resetWindowResizedFlag();
        m_renderer.recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        utl::THROW_ERROR("failed to present swap chain image!");
    }
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
