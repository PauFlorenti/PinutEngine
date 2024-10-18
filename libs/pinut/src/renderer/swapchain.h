#pragma once

namespace Pinut
{
struct SwapchainInfo
{
    VkSwapchainKHR     swapchain{VK_NULL_HANDLE};
    VkSurfaceFormatKHR surfaceFormat{};
    VkExtent2D         surfaceExtent{};

    std::vector<VkImage>     images;
    std::vector<VkImageView> imageViews;
    u32                      imageIndex{0};
    bool                     vsyncEnabled{true};
};
} // namespace Pinut
