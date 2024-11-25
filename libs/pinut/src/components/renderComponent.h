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
    entt::entity id{entt::null};
    // Material
    // Pipeline
};
} // namespace Component
} // namespace Pinut
