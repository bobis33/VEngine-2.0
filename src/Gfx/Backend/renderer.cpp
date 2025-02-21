#include "VEngine/Gfx/Backend/Renderer.hpp"

struct PushConstantData {
    int textureIndex  = 0;
};

void ven::Renderer::recordCommandBuffer(const VkCommandBuffer commandBuffer, const uint32_t imageIndex, const VkPipeline& graphicsPipeline, const VkBuffer& vertexBuffer, const VkBuffer& indexBuffer, const VkPipelineLayout& pipelineLayout, const uint32_t indiceSize, const VkDescriptorSet* descriptorSets, const std::vector<Model>& models) const {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw utl::THROW_ERROR("failed to begin recording command buffer!");
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_swapChain.getRenderPass();
    renderPassInfo.framebuffer = m_swapChain.getSwapChainFrameBuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {.x=0, .y=0};
    renderPassInfo.renderArea.extent = m_swapChain.getExtent();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearValues.size());
    renderPassInfo.pClearValues = m_clearValues.data();
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    const VkViewport viewport{.x=0.0F, .y=0.0F, .width=static_cast<float>(m_swapChain.getExtent().width), .height=static_cast<float>(m_swapChain.getExtent().height), .minDepth=0.0F, .maxDepth=1.0F };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    const VkRect2D scissor{ .offset = {.x=0, .y=0}, .extent = m_swapChain.getExtent() };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    const std::array vertexBuffers = {vertexBuffer};
    constexpr std::array<VkDeviceSize, 1> offsets = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers.data(), offsets.data());
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSets, 0, nullptr);
    vkCmdDrawIndexed(commandBuffer, indiceSize, 1, 0, 0, 0);
    m_gui.render(commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw utl::THROW_ERROR("failed to record command buffer!");
    }
}

void ven::Renderer::createCommandBuffers() {
    m_commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
    if (vkAllocateCommandBuffers(m_device.getVkDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        throw utl::THROW_ERROR("failed to allocate command buffers!");
    }
}

void ven::Renderer::recreateSwapChain() {
    int width = 0;
    int height = 0;
    m_window.getFrameBufferSize(width, height);
    while (width == 0 || height == 0) {
        m_window.getFrameBufferSize(width, height);
        Window::waitEvents();
    }
    m_device.waitIdle();
    m_swapChain.cleanupSwapChain();
    m_swapChain.init();
}