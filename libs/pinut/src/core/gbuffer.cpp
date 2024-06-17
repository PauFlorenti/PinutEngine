// Some comment

#include "stdafx.h"

#include "gbuffer.h"

#include "Device.h"

namespace Pinut
{
    void GBuffer::OnCreate(Device *inDevice, GBufferDescriptor inGBufferDescriptor)
    {
        device = inDevice;

        flags = NONE;
        for (auto &descriptor : inGBufferDescriptor)
        {
            flags = (GBufferFlagBits)(flags | descriptor.first);
        }

        formats = std::move(inGBufferDescriptor);
    }

    void GBuffer::OnDestroy()
    {
        DestroyDisplayDependantResources();
    }

    void GBuffer::CreateDisplayDependantResources(const uint32_t width, const uint32_t height)
    {
        if (flags & DEPTH)
        {
            depth.CreateRenderTarget(device, width, height, formats[DEPTH], VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "DepthGBuffer");
        }
        if (flags & DIFFUSE)
        {
            diffuse.CreateRenderTarget(device, width, height, formats[DIFFUSE], VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, "DiffuseGBuffer");
        }
        if (flags & NORMAL)
        {
            normal.CreateRenderTarget(device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "NormalGBuffer");
        }
        if (flags & METALLIC_ROUGHNESS)
        {
            metallicRoughness.CreateRenderTarget(device, width, height, VK_FORMAT_R8G8B8A8_SRGB, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, "MetallicRoughnessGBuffer");
        }
    }

    void GBuffer::DestroyDisplayDependantResources()
    {
        diffuse.Destroy();
        normal.Destroy();
        metallicRoughness.Destroy();
        depth.Destroy();
    }

    void GBuffer::GetAttachments(GBufferFlagBits attachmentFlags, std::vector<VkImageView> *attachments, std::vector<VkClearValue> *clearValues) const
    {
        attachments->clear();

        // TODO Make clear values optional.

        if (attachmentFlags & DEPTH)
        {
            attachments->push_back(depth.GetImageView());
            VkClearValue clear_value{
                .depthStencil{1.0f, 0},
            };
            clearValues->push_back(clear_value);
        }

        if (attachmentFlags & NORMAL)
        {
            attachments->push_back(normal.GetImageView());
            clearValues->push_back({0.0f, 1.0f, 0.0f, 0.0f});
        }

        if (attachmentFlags & DIFFUSE)
        {
            attachments->push_back(diffuse.GetImageView());
            clearValues->push_back({0.2f, 0.1f, 0.05f, 0.0f});
        }

        if (attachmentFlags & METALLIC_ROUGHNESS)
        {
            attachments->push_back(metallicRoughness.GetImageView());
            clearValues->push_back({0.0f, 0.0f, 1.0f, 0.0f});
        }
    }
    void GBuffer::GetDepthBuffer(VkImageView &imageView, VkClearValue &clearValue) const
    {
        if (auto view = depth.GetImageView())
        {
            imageView = view;
            clearValue = {1.0f, 0};
        }
    }
}
