#pragma once

#include <map>

namespace Pinut
{
class Asset;
class Device;
class Mesh;
class Texture;
class AssetManager
{
    friend Mesh;
    friend Texture;

  public:
    AssetManager()  = default;
    ~AssetManager() = default;

    static AssetManager* Get();

    void Init(Device* device);
    void Shutdown();

    void LoadAsset(const std::string& filename, const std::string& name);
    void RegisterAsset(const std::string& name, std::shared_ptr<Asset> asset);
    void ReleaseAsset(const std::string& name);

    template <typename T>
    std::shared_ptr<T> GetAsset(const std::string& name)
    {
        assert(!name.empty());
        const auto it = m_assets.find(name);

        if (it != m_assets.end())
        {
            std::shared_ptr<Asset> asset        = it->second;
            std::shared_ptr<T>     assetDerived = std::dynamic_pointer_cast<T>(asset);
            if (assetDerived)
                return assetDerived;
        }

        printf("[ERROR]: Asset not registered in AssetManager. Could not retrieve it.");
        return nullptr;
    }

  private:
    void LoadMesh(const std::string& filename, const std::string& name);

    Device*                                       m_device{nullptr};
    static AssetManager*                          m_managerInstance;
    std::map<std::string, std::shared_ptr<Asset>> m_assets;
};
} // namespace Pinut
