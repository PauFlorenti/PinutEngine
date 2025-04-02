#pragma once

#include <common/color.h>

#include "pinut/assets/asset.h"
#include "pinut/assets/texture.h"

namespace Pinut
{
class Material final : public Asset
{
  public:
    Material() = delete;
    Material(const std::string& InFilepath);
    ~Material() = default;

    void Destroy() override;

    common::Color m_diffuse = common::Color::White();
    u32           specularExponent{1};
    common::Color m_specular = common::Color::Black();
    common::Color m_emissive = common::Color::White();

    std::string m_diffuseTexture;
    std::string m_specularTexture;
    std::string m_normalTexture;
    std::string m_metallicRoughnessTexture;
    std::string m_emissiveTexture;

    // Texture difuseTexture;
    // Texture normalTexture;
    // Texture metallicRoughnessTexture;
    // Texture emissiveTexture;
};
} // namespace Pinut
