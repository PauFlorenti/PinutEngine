#pragma once

namespace RED
{
struct TextureDescriptor
{
    VkExtent3D        extent;
    VkFormat          format;
    VkImageUsageFlags usage;
};
} // namespace RED
