#pragma once

#include "render_device/texture.h"

namespace RED
{
struct AttachmentSet
{
    std::array<GPUTextureView, 4>
                   attachments; // TODO Create constant to express maximum number of attachments.
    GPUTextureView depthAttachment;
};
} // namespace RED
