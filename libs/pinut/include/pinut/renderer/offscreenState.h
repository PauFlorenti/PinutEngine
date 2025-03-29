#pragma once

#include "render_device/buffer.h"
#include "render_device/texture.h"
#include "render_device/textureFormat.h"

namespace RED
{
class Device;
}
namespace Pinut
{
struct OffscreenState
{
    void Create(RED::Device&                           device,
                u32                                    width,
                u32                                    height,
                const std::vector<RED::TextureFormat>& attachmentFormats,
                bool                                   depth,
                RED::TextureFormat                     depthFormat);

    void Clear();

    u32 width;
    u32 height;

    RED::GPUBuffer                 globalUniformBuffer;
    RED::GPUBuffer                 lightsBuffer;
    RED::GPUBuffer                 quadBuffer;
    RED::GPUTexture                depthTexture;
    std::array<RED::GPUTexture, 4> colorTextures;
};
} // namespace Pinut
