#pragma once

#include "render_device/textureFormat.h"

namespace RED
{
struct TextureDescriptor
{
    u32               width;
    u32               height;
    u32               depth;
    TextureFormat     format{TextureFormat::UNDEFINED};
    VkImageLayout     layout{VK_IMAGE_LAYOUT_UNDEFINED};
    VkImageUsageFlags usage{VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT};
};
} // namespace RED
