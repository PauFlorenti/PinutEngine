#pragma once

#include <filesystem>
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

    void RegisterAsset(const std::string& name, std::shared_ptr<Asset> asset);
    void ReleaseAsset(const std::string& name);

    template <typename T>
    std::shared_ptr<T> GetAsset(std::filesystem::path filename)
    {
        assert(!filename.empty());

        const auto name = filename.filename().string();
        const auto it   = m_assets.find(name);

        if (it != m_assets.end())
        {
            if (auto assetDerived = std::dynamic_pointer_cast<T>(it->second))
                return std::make_shared<T>(*assetDerived);
        }

        // Try load asset.
        if (auto assetDerived = std::dynamic_pointer_cast<T>(LoadAsset(std::move(filename), name)))
            return assetDerived;

        printf("[ERROR]: Asset not registered in AssetManager. Could not retrieve it.");
        return nullptr;
    }

  private:
    std::shared_ptr<Asset> LoadAsset(std::filesystem::path filename, const std::string& name);
    std::shared_ptr<Mesh>  LoadMesh(std::filesystem::path filename, const std::string& name);

    Device*                                       m_device{nullptr};
    std::shared_ptr<MaterialManager>              m_materialManager;
    std::map<std::string, std::shared_ptr<Asset>> m_assets;
    std::filesystem::path                         m_assetsPath;
};
} // namespace Pinut
