#pragma once

#include <render_device/buffer.h>
#include <render_device/texture.h>

namespace RED
{
class Device;
}
namespace Pinut
{
class Material;
void CreateMaterialData(std::shared_ptr<RED::Device> InDevice,
                        entt::registry&              InRegistry,
                        std::shared_ptr<Material>    InMaterial);
struct MaterialData
{
    using TextureId = entt::entity;

    TextureId      difuseTexture;
    TextureId      normalTexture;
    TextureId      metallicRoughnessTexture;
    TextureId      emissiveTexture;
    RED::GPUBuffer uniformBuffer;
    RED::GPUBuffer modelBuffer;
};
} // namespace Pinut
