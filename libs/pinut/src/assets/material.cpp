#include "pch.hpp"

#include "pinut/assets/asset.h"
#include "pinut/assets/material.h"

namespace Pinut
{
Material::Material(const std::string& InName) : Asset(InName, AssetType::MATERIAL) {};

void Material::Destroy() {}
} // namespace Pinut
