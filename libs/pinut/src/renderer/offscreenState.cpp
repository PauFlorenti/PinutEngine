#include "stdafx.h"

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"
#include "render_device/textureDescriptor.h"

#include "src/renderer/offscreenState.h"

namespace Pinut
{
void OffscreenState::Create(RED::Device&                 device,
                            u32                          width,
                            u32                          height,
                            const std::vector<VkFormat>& attachmentFormats,
                            bool                         depth,
                            VkFormat                     depthFormat)
{
    RED::TextureDescriptor descriptor;
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

    globalUniformBuffer = device.CreateBuffer({140, 140, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT});

    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    // clang-format off
    std::array<QuadVertex, 6> quadData = {QuadVertex{glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
                                          QuadVertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
                                          QuadVertex{glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
                                          QuadVertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
                                          QuadVertex{glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
                                          QuadVertex{glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f)}};
    // clang-format on
    quadBuffer = device.CreateBuffer({120, 20, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT}, quadData.data());
}

void OffscreenState::Clear()
{
    depthTexture.Destroy();

    for (auto& texture : colorTextures)
    {
        texture.Destroy();
    }

    globalUniformBuffer.Destroy();
    quadBuffer.Destroy();

    width  = 0;
    height = 0;
}
} // namespace Pinut
