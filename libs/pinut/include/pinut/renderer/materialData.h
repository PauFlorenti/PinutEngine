#pragma once

#include <render_device/texture.h>

namespace Pinut
{
class Material;
void CreateMaterialData(std::shared_ptr<RED::Device> InDevice,
                        entt::registry&              InRegistry,
                        std::shared_ptr<Material>    InMaterial);
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
