#pragma once

#include "render_device/texture.h"

namespace RED
{
class Device;
struct OffscreenState
{
    void Create(Device&                      device,
                u32                          width,
                u32                          height,
                const std::vector<VkFormat>& attachmentFormats,
                bool                         depth,
                VkFormat                     depthFormat);

    void Clear();

    u32 width;
    u32 height;

    GPUTexture                depthTexture;
    std::array<GPUTexture, 4> colorTextures;
};
} // namespace RED
