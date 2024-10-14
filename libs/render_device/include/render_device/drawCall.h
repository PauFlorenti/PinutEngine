#pragma once

#include "render_device/buffer.h"

namespace RED
{
struct DrawCall
{
    GPUBuffer vertexBuffer;
    GPUBuffer indexBuffer;
    u32       vertexCount{0};
    u32       indexCount{0};
};
} // namespace RED
