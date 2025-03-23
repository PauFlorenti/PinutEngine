#include "stdafx.h"

#include "src/assets/asset.h"
#include "src/assets/material.h"

namespace Pinut
{
Material::Material(const std::string& InName) : Asset(InName, AssetType::MATERIAL) {};

void Material::Destroy() {}
} // namespace Pinut
