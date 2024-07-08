#include "stdafx.h"

#include "material.h"
#include "texture.h"

namespace Pinut
{
void Material::Destroy()
{
    if (m_albedo)
        m_albedo->Destroy();

    if (m_normal)
        m_normal->Destroy();

    if (m_metalicRoughness)
        m_metalicRoughness->Destroy();
}

void Material::SetAlbedo(Texture* t)
{
    assert(t);
    m_albedo = t;
}

void Material::SetNormal(Texture* t)
{
    assert(t);
    m_normal = t;
}

void Material::SetMetallicRoughness(Texture* t)
{
    assert(t);
    m_metalicRoughness = t;
}

} // namespace Pinut
