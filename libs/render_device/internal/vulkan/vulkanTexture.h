#pragma once

#include "render_device/textureDescriptor.h"
#include "render_device/textureFormat.h"

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
