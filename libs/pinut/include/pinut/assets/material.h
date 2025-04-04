#pragma once

#include <common/color.h>

#include "pinut/assets/asset.h"
#include "pinut/assets/texture.h"

namespace Pinut
{
class Renderer;
class Material final : public Asset
{
  public:
    Material() = delete;
    Material(const std::string& InFilepath);
    ~Material() = default;

    friend Renderer;

    void Destroy() override;

    common::Color m_diffuse = common::Color::White();
    u32           specularExponent{1};
    common::Color m_specular = common::Color::Black();
    common::Color m_emissive = common::Color::White();

    std::shared_ptr<Texture> diffuseTexture;
    std::shared_ptr<Texture> normalTexture;
    std::shared_ptr<Texture> metallicRoughnessTexture;
    std::shared_ptr<Texture> emissiveTexture;

  private:
    entt::entity m_handle;
};
} // namespace Pinut
