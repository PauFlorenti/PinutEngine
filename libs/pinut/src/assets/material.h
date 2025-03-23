#pragma once

#include "src/assets/asset.h"
namespace Pinut
{
class Material final : public Asset
{
  public:
    Material() = delete;
    Material(const std::string& InFilepath);
    ~Material() = default;

    void Destroy() override;

    glm::vec3 m_diffuse{};
    glm::vec3 m_specular{};
    glm::vec3 m_emissive{};

    std::string m_diffuseTexture;
    std::string m_specularTexture;
    std::string m_normalTexture;
    std::string m_metallicRoughnessTexture;
    std::string m_emissiveTexture;
};
} // namespace Pinut
