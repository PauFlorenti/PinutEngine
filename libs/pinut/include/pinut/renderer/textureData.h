#pragma once

#include <render_device/texture.h>

namespace Pinut
{

void CreateTextureData(std::shared_ptr<RED::Device> InDevice,
                       entt::registry&              InRegistry,
                       std::shared_ptr<Texture>     InTexture);

struct TextureData
{
    friend Renderer;

    RED::GPUTexture texture;
};
} // namespace Pinut
