//#pragma once
//
//#include "asset.h"
//namespace Pinut
//{
//// class Device;
//class Texture final : public Asset
//{
//  public:
//    static VkImageSubresourceRange GetImageSubresourceRange(VkImageAspectFlags aspectMaskFlags);
//    static void                    TransitionImageLayout(VkCommandBuffer         cmd,
//                                                         VkImage                 image,
//                                                         VkAccessFlags           srcAccessFlags,
//                                                         VkAccessFlags           dstAccessFlags,
//                                                         VkImageLayout           currentLayout,
//                                                         VkImageLayout           targetLayout,
//                                                         VkPipelineStageFlags    srcStageFlags,
//                                                         VkPipelineStageFlags    dstStageFlags,
//                                                         VkImageSubresourceRange subresourceRange);
//    static void                    CopyTextureToTexture(VkCommandBuffer cmd,
//                                                        VkImage         src,
//                                                        VkImage         dst,
//                                                        VkExtent2D      srcExtent,
//                                                        VkExtent2D      dstExtent);
//
//    ~Texture() = default;
//
//    static std::shared_ptr<Texture> CreateFromData(const u32         width,
//                                                   const u32         height,
//                                                   const u32         channels,
//                                                   VkFormat          format,
//                                                   VkImageUsageFlags usage,
//                                                   void*             data,
//                                                   Device*           device);
//    static std::shared_ptr<Texture> CreateFromFile(const std::string& filename, Device* device);
//
//    Texture(Device* device, const VkImageCreateInfo& info);
//    void Destroy() override;
//
//    VkImage     Image() const { return m_image; }
//    VkImageView ImageView() const { return m_imageView; }
//    VkSampler   Sampler() const { return m_sampler; }
//
//    void SetSampler(VkSampler s) { m_sampler = std::move(s); }
//
//  private:
//    VkImage       m_image{VK_NULL_HANDLE};
//    VkImageView   m_imageView{VK_NULL_HANDLE};
//    VkFormat      m_format{};
//
//    // Temp ...
//    VkSampler m_sampler{VK_NULL_HANDLE};
//
//    u32 m_width{0};
//    u32 m_height{0};
//};
//} // namespace Pinut
