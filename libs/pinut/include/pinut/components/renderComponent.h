#pragma once

#include "pinut/assets/texture.h"
#include "pinut/components/baseComponent.h"

namespace Pinut
{
class Material;
namespace Component
{
struct RenderComponent : public BaseComponent
{
    RenderComponent(Texture inDifuse,
                    Texture inNormal,
                    Texture inMetallicRoughness,
                    Texture inEmissive);

    RenderComponent(std::shared_ptr<Material> InMaterial);

#ifdef _DEBUG
    void RenderDebug() override;
#endif

    Texture difuse;
    Texture normal;
    Texture metallicRoughness;
    Texture emissive;

    std::shared_ptr<Material> material;

    entt::entity m_handle{entt::null};
};
} // namespace Component
} // namespace Pinut
