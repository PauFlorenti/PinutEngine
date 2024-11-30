#pragma once

#include "render_device/texture.h"

namespace Pinut
{
struct MaterialData
{
    RED::GPUTexture difuseTexture;
    RED::GPUBuffer  uniformBuffer;
};
} // namespace Pinut
