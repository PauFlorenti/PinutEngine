#pragma once

#include "pinut/components/baseComponent.h"

namespace Pinut
{
class Material;
class Renderer;
class Texture;
namespace Component
{
struct RenderComponent : public BaseComponent
{
    using TexturePtr  = std::shared_ptr<Texture>;
    using MaterialPtr = std::shared_ptr<Material>;

    RenderComponent(MaterialPtr InMaterial);

    friend Renderer;

    // BaseComponent
#ifdef _DEBUG
    void RenderDebug() override;
#endif

    MaterialPtr material;
};
} // namespace Component
} // namespace Pinut
