#include "stdafx.h"

#include "src/renderer/device.h"
#include "texture.h"

namespace Pinut
{
VkImageSubresourceRange Texture::GetImageSubresourceRange(VkImageAspectFlags aspectMaskFlags)
{
    return VkImageSubresourceRange{.aspectMask     = aspectMaskFlags,
                                   .baseMipLevel   = 0,
                                   .levelCount     = 1,
                                   .baseArrayLayer = 0,
                                   .layerCount     = 1};
}

void Texture::TransitionImageLayout(VkCommandBuffer         cmd,
                                    VkImage                 image,
                                    VkAccessFlags           srcAccessFlags,
                                    VkAccessFlags           dstAccessFlags,
                                    VkImageLayout           currentLayout,
                                    VkImageLayout           targetLayout,
                                    VkPipelineStageFlags    srcStageFlags,
                                    VkPipelineStageFlags    dstStageFlags,
                                    VkImageSubresourceRange subresourceRange)
{
    VkImageMemoryBarrier2 barrier{
      .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
      .srcStageMask     = srcStageFlags,
      .srcAccessMask    = srcAccessFlags,
      .dstStageMask     = dstStageFlags,
      .dstAccessMask    = dstAccessFlags,
      .oldLayout        = currentLayout,
      .newLayout        = targetLayout,
      .image            = image,
      .subresourceRange = subresourceRange,
    };

    VkDependencyInfo dependency{
      .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
      .imageMemoryBarrierCount = 1,
      .pImageMemoryBarriers    = &barrier,
    };

    vkCmdPipelineBarrier2(cmd, &dependency);
}

void Texture::CopyTextureToTexture(VkCommandBuffer cmd,
                                   VkImage         src,
                                   VkImage         dst,
                                   VkExtent2D      srcExtent,
                                   VkExtent2D      dstExtent)
{
    VkImageBlit2 blitRegion{.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr};

    blitRegion.srcOffsets[1].x = srcExtent.width;
    blitRegion.srcOffsets[1].y = srcExtent.height;
    blitRegion.srcOffsets[1].z = 1;

    blitRegion.dstOffsets[1].x = dstExtent.width;
    blitRegion.dstOffsets[1].y = dstExtent.height;
    blitRegion.dstOffsets[1].z = 1;

    blitRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.srcSubresource.baseArrayLayer = 0;
    blitRegion.srcSubresource.layerCount     = 1;
    blitRegion.srcSubresource.mipLevel       = 0;

    blitRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    blitRegion.dstSubresource.baseArrayLayer = 0;
    blitRegion.dstSubresource.layerCount     = 1;
    blitRegion.dstSubresource.mipLevel       = 0;

    VkBlitImageInfo2 blitInfo{.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr};
    blitInfo.dstImage       = dst;
    blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    blitInfo.srcImage       = src;
    blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    blitInfo.filter         = VK_FILTER_LINEAR;
    blitInfo.regionCount    = 1;
    blitInfo.pRegions       = &blitRegion;

    vkCmdBlitImage2(cmd, &blitInfo);
}

void Texture::Create(Device* device, const VkImageCreateInfo& info)
{
    assert(device);
    m_device = device;

    m_width  = info.extent.width;
    m_height = info.extent.height;
    m_format = info.format;

    VmaAllocationCreateInfo allocationCreateInfo{
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    VmaAllocationInfo allocationInfo;

    assert(vmaCreateImage(m_device->GetAllocator(),
                          &info,
                          &allocationCreateInfo,
                          &m_image,
                          &m_allocation,
                          &allocationInfo) == VK_SUCCESS);

    VkImageSubresourceRange subresourceRange{};
    if (m_format == VK_FORMAT_D32_SFLOAT)
        subresourceRange = GetImageSubresourceRange(VK_IMAGE_ASPECT_DEPTH_BIT);
    else if (m_format == VK_FORMAT_D32_SFLOAT_S8_UINT)
        subresourceRange =
          GetImageSubresourceRange(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
    else
        subresourceRange = GetImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageViewCreateInfo viewInfo{
      .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext            = nullptr,
      .image            = m_image,
      .viewType         = VK_IMAGE_VIEW_TYPE_2D,
      .format           = m_format,
      .subresourceRange = subresourceRange,
    };

    auto ok = vkCreateImageView(device->GetDevice(), &viewInfo, nullptr, &m_imageView);
    assert(ok == VK_SUCCESS);
}

void Texture::Destroy()
{
    vmaDestroyImage(m_device->GetAllocator(), m_image, m_allocation);
    if (m_imageView)
        vkDestroyImageView(m_device->GetDevice(), m_imageView, nullptr);
}

} // namespace Pinut