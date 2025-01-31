#pragma once

#include <entt/entt.hpp>

#include "src/assets/texture.h"

namespace Pinut
{
namespace Component
{
struct RenderComponent
{
    Texture      difuse;
    Texture      normal;
    Texture      metallicRoughness;
    Texture      emissive;
    entt::entity id{entt::null};
    // Material
    // Pipeline
};
} // namespace Component
} // namespace Pinut
