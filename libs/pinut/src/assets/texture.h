#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "asset.h"
namespace Pinut
{
class Device;
class Texture final : public Asset
{
  public:
    static VkImageSubresourceRange GetImageSubresourceRange(VkImageAspectFlags aspectMaskFlags);
    static void                    TransitionImageLayout(VkCommandBuffer         cmd,
                                                         VkImage                 image,
                                                         VkAccessFlags           srcAccessFlags,
                                                         VkAccessFlags           dstAccessFlags,
                                                         VkImageLayout           currentLayout,
                                                         VkImageLayout           targetLayout,
                                                         VkPipelineStageFlags    srcStageFlags,
                                                         VkPipelineStageFlags    dstStageFlags,
                                                         VkImageSubresourceRange subresourceRange);
    static void                    CopyTextureToTexture(VkCommandBuffer cmd,
                                                        VkImage         src,
                                                        VkImage         dst,
                                                        VkExtent2D      srcExtent,
                                                        VkExtent2D      dstExtent);

    void Create(Device* device, const VkImageCreateInfo& info);
    void CreateFromData(Device*            device,
                        const u32          width,
                        const u32          height,
                        const u32          channels,
                        VkFormat           format,
                        VkImageUsageFlags  usage,
                        void*              data,
                        const std::string& name = "");
    void CreateFromFile(Device* device, const std::string& filename, const std::string& name = "");
    void Destroy();

    VkImage     Image() const { return m_image; }
    VkImageView ImageView() const { return m_imageView; }
    VkSampler   Sampler() const { return m_sampler; }

  private:
    Device*       m_device{nullptr};
    VkImage       m_image{VK_NULL_HANDLE};
    VkImageView   m_imageView{VK_NULL_HANDLE};
    VmaAllocation m_allocation{nullptr};
    VkFormat      m_format{};

    // Temp ...
    VkSampler m_sampler{VK_NULL_HANDLE};

    u32 m_width{0};
    u32 m_height{0};
};
} // namespace Pinut
