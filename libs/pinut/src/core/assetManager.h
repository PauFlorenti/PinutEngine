#pragma once

#include <map>

namespace Pinut
{
class Asset;
class AssetManager
{
  public:
    AssetManager()  = default;
    ~AssetManager() = default;

    static AssetManager& Get();

    void Init();
    void Shutdown();

    void   LoadAsset(const std::string& filename);
    void   RegisterAsset(const std::string& name, Asset* asset);
    Asset* GetAsset(const std::string& name);
    void   ReleaseAsset(const std::string& name);

  private:
    static AssetManager*                          m_instance;
    std::map<std::string, std::pair<Asset*, u64>> m_assets;
};
} // namespace Pinut
