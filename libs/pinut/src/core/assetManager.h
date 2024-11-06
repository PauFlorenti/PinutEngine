#pragma once

#include <filesystem>
#include <map>

// #include "src/assets/material.h"
#include "src/loaders/objLoader.h"
// #include "src/renderer/stages/materialManager.h"

namespace RED
{
class Device;
}
namespace Pinut
{
class Asset;
class Mesh;
class Renderable;
class Texture;
class AssetManager
{
    friend Mesh;
    friend Texture;

  public:
    AssetManager()  = default;
    ~AssetManager() = default;

    static bool FindFile(const std::filesystem::path& filepath,
                         std::filesystem::path&       outAbsolutePath);

    void Init(std::weak_ptr<RED::Device> device);
    void Shutdown();

    void RegisterAsset(const std::string& name, std::shared_ptr<Asset> asset);
    void ReleaseAsset(const std::string& name);

    // Device* GetDevice() const { return m_device; }

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

        if (filename.extension().empty())
            return nullptr;

        // Try load asset if extension.
        if (auto assetDerived = std::dynamic_pointer_cast<T>(LoadAsset(std::move(filename), name)))
            return assetDerived;

        printf("[ERROR]: Asset not registered in AssetManager. Could not retrieve it.");
        return nullptr;
    }

    static const std::filesystem::path m_assetsPath;

  private:
    std::shared_ptr<Asset> LoadAsset(std::filesystem::path filename, const std::string& name);

    std::weak_ptr<RED::Device> m_device;
    // MaterialManager                               m_materialManager;
    std::unordered_map<std::string, std::shared_ptr<Asset>> m_assets;

    OBJLoader m_objLoader;
};
} // namespace Pinut
