#pragma once

namespace Pinut
{
enum AssetType
{
    MATERIAL = 0,
    MESH,
    TEXTURE,
    COUNT
};

class Asset
{
  public:
    // explicit Asset(std::string name) : m_name(std::move(name)) {}

  private:
    std::string m_name{};
    AssetType   m_type = COUNT;
};
} // namespace Pinut
