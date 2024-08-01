#include "stdafx.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "src/core/assetManager.h"
#include "src/renderer/buffer.h"
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

    VmaAllocationInfo allocationInfo{};

    auto ok = vmaCreateImage(m_device->GetAllocator(),
                             &info,
                             &allocationCreateInfo,
                             &m_image,
                             &m_allocation,
                             &allocationInfo);
    assert(ok == VK_SUCCESS);

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

    ok = vkCreateImageView(device->GetDevice(), &viewInfo, nullptr, &m_imageView);
    assert(ok == VK_SUCCESS);
}

std::shared_ptr<Texture> Texture::CreateFromData(const u32          width,
                                                 const u32          height,
                                                 const u32          channels,
                                                 VkFormat           format,
                                                 VkImageUsageFlags  usage,
                                                 void*              data,
                                                 const std::string& name)
{
    auto       assetManager = AssetManager::Get();
    const auto device       = assetManager->m_device;
    auto       t            = std::make_shared<Texture>();

    VkImageCreateInfo info{
      .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext                 = nullptr,
      .imageType             = VK_IMAGE_TYPE_2D,
      .format                = format,
      .extent                = {width, height, 1},
      .mipLevels             = 1,
      .arrayLayers           = 1,
      .samples               = VK_SAMPLE_COUNT_1_BIT,
      .tiling                = VK_IMAGE_TILING_OPTIMAL,
      .usage                 = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices   = nullptr,
      .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    t->Create(device, info);

    const auto   cmd         = device->CreateImmediateCommandBuffer();
    const size_t textureSize = width * height * channels;
    GPUBuffer    buffer;
    buffer.Create(device, textureSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    memcpy(buffer.AllocationInfo().pMappedData, data, textureSize);

    TransitionImageLayout(cmd,
                          t->Image(),
                          VK_ACCESS_TRANSFER_WRITE_BIT,
                          VK_ACCESS_TRANSFER_READ_BIT,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    VkBufferImageCopy region{
      .bufferOffset = 0,
      .imageSubresource{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .layerCount = 1,
      },
      .imageOffset = 0,
      .imageExtent = info.extent,
    };

    vkCmdCopyBufferToImage(cmd,
                           buffer.m_buffer,
                           t->Image(),
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);

    TransitionImageLayout(cmd,
                          t->Image(),
                          VK_ACCESS_TRANSFER_READ_BIT,
                          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                          VK_PIPELINE_STAGE_TRANSFER_BIT,
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    device->FlushCommandBuffer(cmd);

    buffer.Destroy();

    VkSamplerCreateInfo samplerInfo{
      .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter               = VK_FILTER_LINEAR,
      .minFilter               = VK_FILTER_LINEAR,
      .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias              = 0.0f,
      .anisotropyEnable        = VK_FALSE,
      .compareEnable           = VK_TRUE,
      .compareOp               = VK_COMPARE_OP_ALWAYS,
      .minLod                  = 0.0f,
      .maxLod                  = 0.0f,
      .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
      .unnormalizedCoordinates = VK_FALSE,
    };

    VkSampler sampler;
    auto      ok = vkCreateSampler(device->GetDevice(), &samplerInfo, nullptr, &sampler);
    assert(ok == VK_SUCCESS);

    t->SetSampler(std::move(sampler));

    assetManager->RegisterAsset(name, t);
    return assetManager->GetAsset<Texture>(name);
}

std::shared_ptr<Texture> Texture::CreateFromFile(const std::string& filename,
                                                 const std::string& name)
{
    int32_t  w, h, channels;
    stbi_uc* pixels = stbi_load(filename.c_str(), &w, &h, &channels, STBI_rgb_alpha);

    if (!pixels)
        return nullptr;

    auto t = CreateFromData(w,
                            h,
                            4,
                            VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                            pixels,
                            name);

    stbi_image_free(pixels);

    return t;
}

void Texture::Destroy()
{
    vmaDestroyImage(m_device->GetAllocator(), m_image, m_allocation);
    m_image = VK_NULL_HANDLE;

    if (m_imageView)
    {
        vkDestroyImageView(m_device->GetDevice(), m_imageView, nullptr);
        m_imageView = VK_NULL_HANDLE;
    }

    if (m_sampler)
    {
        vkDestroySampler(m_device->GetDevice(), m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }
}

} // namespace Pinut
