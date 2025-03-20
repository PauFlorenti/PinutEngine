#pragma once

#include <filesystem>
#include <map>

#include "src/loaders/objLoader.h"

namespace Pinut
{
class Asset;
class Mesh;
class Texture;

class AssetManager
{
    friend Mesh;
    friend Texture;

  public:
    AssetManager();
    ~AssetManager();

    static const std::filesystem::path m_assetsPath;

    void ImportAsset(const std::filesystem::path& InFilepath);

    template <typename T>
    std::shared_ptr<T> GetAsset(std::filesystem::path InFilename) const
    {
        assert(!InFilename.empty());
        assert(!InFilename.extension().empty());

        const std::string filename = (m_assetsPath / InFilename).string();
        const auto        guid     = std::hash<std::string>{}(filename);

        // Return asset if already loaded.
        if (const auto it = m_assets.find(guid); it != m_assets.end())
        {
            if (auto assetDerived = std::dynamic_pointer_cast<T>(it->second))
            {
                return std::make_shared<T>(*assetDerived);
            }

            return nullptr;
        }

        printf("[ERROR]: Asset not registered in AssetManager. Could not retrieve it.");
        return nullptr;
    }

  private:
    static bool FindFile(const std::filesystem::path& filepath,
                         std::filesystem::path&       outAbsolutePath);

    std::hash<std::string>                             m_hasher;
    std::unordered_map<size_t, std::shared_ptr<Asset>> m_assets;

    OBJLoader m_objLoader;
};
} // namespace Pinut
