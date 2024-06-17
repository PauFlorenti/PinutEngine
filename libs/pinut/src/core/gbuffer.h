#pragma once

#include "vulkan/vulkan.h"
#include "texture.h"

namespace Pinut
{
    class Device;

    enum GBufferFlagBits
    {
        NONE = 0,
        DEPTH = 1,
        NORMAL = 2,
        DIFFUSE = 4,
        METALLIC_ROUGHNESS = 8,
    };

    class GBuffer
    {
    public:
        using GBufferDescriptor = std::map<GBufferFlagBits, VkFormat>;
        void OnCreate(Device *inDevice, GBufferDescriptor inGBufferDescriptor);
        void OnDestroy();

        void CreateDisplayDependantResources(const uint32_t width, const uint32_t height);
        void DestroyDisplayDependantResources();

        void GetAttachments(GBufferFlagBits attachmentFlags, std::vector<VkImageView> *attachments, std::vector<VkClearValue> *clearValues) const;
        void GetDepthBuffer(VkImageView& imageView, VkClearValue& clearValue) const;

        Texture depth;
        Texture diffuse;
        Texture normal;
        Texture metallicRoughness;
        Texture HDR;

    private:
        Device *device{nullptr};
        GBufferFlagBits flags;
        GBufferDescriptor formats;
    };
}