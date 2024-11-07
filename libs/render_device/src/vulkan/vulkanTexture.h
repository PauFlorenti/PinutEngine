#pragma once

#include "render_device/textureDescriptor.h"

namespace RED
{
namespace vulkan
{
struct VulkanTexture
{
    VkImage           image;
    VkImageView       imageView;
    VmaAllocation     allocation;
    TextureDescriptor descriptor;
};
} // namespace vulkan
} // namespace RED
