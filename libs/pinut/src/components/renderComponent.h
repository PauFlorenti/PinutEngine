#pragma once

#include "src/assets/texture.h"
#include "src/components/baseComponent.h"

namespace Pinut
{
namespace Component
{
struct RenderComponent : public BaseComponent
{
    RenderComponent(Texture inDifuse,
                    Texture inNormal,
                    Texture inMetallicRoughness,
                    Texture inEmissive);

#ifdef _DEBUG
    void RenderDebug() override;
#endif

    Texture      difuse;
    Texture      normal;
    Texture      metallicRoughness;
    Texture      emissive;
    entt::entity m_handle{entt::null};
};
} // namespace Component
} // namespace Pinut
