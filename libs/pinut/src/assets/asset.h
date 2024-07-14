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
    virtual ~Asset() = default;
};
} // namespace Pinut
