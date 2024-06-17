#pragma once

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.h"

namespace Pinut
{
    class Device;

    class Texture
    {
    public:
        static VkImageSubresourceRange GetImageSubresourceRange(VkImageAspectFlags aspectMaskFlags);
        static void TransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout targetLayout);
        static void CopyTextureToTexture(VkCommandBuffer cmd, VkImage src, VkImage dst, VkExtent2D srcExtent, VkExtent2D dstExtent);

        void Create(Device *inDevice, VkImageCreateInfo *createInfo, const char *inName);
        void CreateRenderTarget(Device *inDevice, uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits msaa, VkImageUsageFlags usageFlags, const char *name, VkImageCreateFlagBits imageCreateFlags = (VkImageCreateFlagBits)0);
        void CreateFromData(Device *inDevice, uint32_t width, uint32_t height, VkFormat format, void *data, const char *name);
        void CreateFromFile(Device *inDevice, const char *filename, const char *name);
        void Destroy();

        const std::string &GetName() const { return name; }
        VkImage GetImage() const { return image; }
        VkImageView GetImageView() const { return imageView; }
        VkSampler GetSampler() const { return sampler; }
        uint32_t GetWidth() const { return width; }
        uint32_t GetHeight() const { return height; }
        uint32_t GetDepth() const { return depth; }

    private:
        std::string name{""};

        Device *device{nullptr};
        VkImage image{VK_NULL_HANDLE};
        VkImageView imageView{VK_NULL_HANDLE};
        VkFormat format{VK_FORMAT_UNDEFINED};
        VmaAllocation allocator{VK_NULL_HANDLE};
        VkSampler sampler{VK_NULL_HANDLE};

        uint32_t width;
        uint32_t height;
        uint32_t depth;
    };
}
