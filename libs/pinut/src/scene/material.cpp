// Some comments
#include "stdafx.h"

#include "material.h"

#include "src/core/texture.h"

namespace Pinut
{
    void Material::Destroy()
    {
        albedo->Destroy();
        normal->Destroy();
        metallicRoughness->Destroy();
    }
}
