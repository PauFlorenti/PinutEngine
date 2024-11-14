#pragma once

namespace RED
{
struct TextureDescriptor
{
    VkExtent3D        extent{};
    VkFormat          format{VK_FORMAT_UNDEFINED};
    VkImageLayout     layout{VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageUsageFlags usage{VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
};
} // namespace RED
