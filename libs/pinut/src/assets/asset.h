#pragma once

namespace Pinut
{
enum class AssetType
{
    MATERIAL = 0,
    MESH,
    TEXTURE,
    COUNT
};

class Asset
{
  public:
    virtual ~Asset()  = default;
    virtual void Destroy() = 0;
};
} // namespace Pinut
