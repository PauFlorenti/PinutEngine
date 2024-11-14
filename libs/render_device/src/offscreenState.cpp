#include "render_device/offscreenState.h"
#include "render_device/device.h"
#include "render_device/textureDescriptor.h"

namespace RED
{
void OffscreenState::Create(Device&                      device,
                            u32                          width,
                            u32                          height,
                            const std::vector<VkFormat>& attachmentFormats,
                            bool                         depth,
                            VkFormat                     depthFormat)
{
    TextureDescriptor descriptor;
    descriptor.extent.width  = width;
    descriptor.extent.height = height;
    descriptor.extent.depth  = 1;

    if (depth)
    {
        descriptor.format = depthFormat;
        descriptor.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        descriptor.usage  = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depthTexture      = device.CreateTexture(descriptor);
    }

    for (size_t attachmentIndex = 0; attachmentIndex < attachmentFormats.size(); ++attachmentIndex)
    {
        descriptor.format = attachmentFormats.at(attachmentIndex);
        descriptor.usage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        descriptor.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorTextures.at(attachmentIndex) = device.CreateTexture(descriptor);
    }
}

void OffscreenState::Clear()
{
    depthTexture.Destroy();

    for (auto& texture : colorTextures)
    {
        texture.Destroy();
    }

    width  = 0;
    height = 0;
}
} // namespace RED
