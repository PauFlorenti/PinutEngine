#pragma once

#include <map>

namespace Pinut
{
class Asset;
class Device;
class MaterialManager;
class Mesh;
class Texture;
class AssetManager
{
    friend Mesh;
    friend Texture;

  public:
    AssetManager()  = default;
    ~AssetManager() = default;

    void Init(Device* device, std::shared_ptr<MaterialManager> materialManager);
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
                return std::make_shared<T>(*assetDerived);
        }

        printf("[ERROR]: Asset not registered in AssetManager. Could not retrieve it.");
        return nullptr;
    }

  private:
    void LoadMesh(const std::string& filename, const std::string& name);

    Device*                                       m_device{nullptr};
    std::shared_ptr<MaterialManager>              m_materialManager;
    std::map<std::string, std::shared_ptr<Asset>> m_assets;
};
} // namespace Pinut
