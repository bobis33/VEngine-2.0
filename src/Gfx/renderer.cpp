#include "VEngine/Gfx/Renderer.hpp"

void ven::Renderer::createCommandBuffers() {
    m_commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_device.getCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
    if (vkAllocateCommandBuffers(m_device.getVkDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
        utl::THROW_ERROR("failed to allocate command buffers!");
    }
}

void ven::Renderer::recreateSwapChain() {
    int width = 0;
    int height = 0;
    m_window.getFramebufferSize(width, height);
    while (width == 0 || height == 0) {
        m_window.getFramebufferSize(width, height);
        ven::Window::waitEvents();
    }
    m_device.waitIdle();
    m_swapChain.cleanupSwapChain();
    m_swapChain.init();
}