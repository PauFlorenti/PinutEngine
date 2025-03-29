#include "pch.hpp"

#include <combaseapi.h>

#include "pinut/assets/asset.h"

namespace Pinut
{
Asset::Asset(const std::string& InName, AssetType InType)
: m_name(InName),
  m_type(InType),
  m_id(std::hash<std::string>{}(InName))
{
}

Asset::~Asset() = default;

const std::string& Asset::GetName() const { return m_name; }
const AssetType    Asset::GetType() const { return m_type; }
const UUID         Asset::GetUUID() const { return m_id; }

} // namespace Pinut
