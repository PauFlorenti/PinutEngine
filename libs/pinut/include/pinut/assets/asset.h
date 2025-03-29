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

using UUID = size_t;

class Asset
{
  public:
    explicit Asset(const std::string& InName, AssetType InType);
    virtual ~Asset();
    virtual void Destroy() = 0;

    const std::string& GetName() const;
    const AssetType    GetType() const;
    const UUID         GetUUID() const;

  private:
    std::string m_name;
    AssetType   m_type;
    UUID        m_id;
};
} // namespace Pinut
