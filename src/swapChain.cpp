#include "VEngine/SwapChain.hpp"
#include "Utils/ErrorHandling.hpp"

void ven::SwapChain::createImageView(const VkDevice& device, const VkImage& image, const VkFormat format, const VkImageAspectFlags aspectFlags, const uint32_t mipLevels, VkImageView& imageView)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        THROW_ERROR("failed to create image view");
    }
}
