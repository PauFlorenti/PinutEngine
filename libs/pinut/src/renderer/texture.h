#pragma once

#include <vulkan/vulkan.h>

namespace Pinut
{
class Texture
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

  private:
};
} // namespace Pinut
