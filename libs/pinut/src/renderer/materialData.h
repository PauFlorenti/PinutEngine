#pragma once

#include "render_device/texture.h"

namespace Pinut
{
struct MaterialData
{
    RED::GPUTexture difuseTexture;
    RED::GPUTexture normalTexture;
    RED::GPUTexture metallicRoughnessTexture;
    RED::GPUTexture emissiveTexture;
    RED::GPUBuffer  uniformBuffer;
    RED::GPUBuffer  modelBuffer;
};
} // namespace Pinut
