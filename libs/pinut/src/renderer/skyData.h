#pragma once

#include "render_device/texture.h"

namespace Pinut
{
struct SkyData
{
    RED::GPUBuffer  skyMeshDataBuffer;
    RED::GPUTexture skyTexture;
    entt::entity    meshHandle{entt::null};
};
} // namespace Pinut
