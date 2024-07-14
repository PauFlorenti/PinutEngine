#include "stdafx.h"

#include "src/assets/asset.h"
#include "src/core/assetManager.h"
#include "src/renderer/device.h"

namespace Pinut
{
AssetManager* AssetManager::m_managerInstance = nullptr;
AssetManager* AssetManager::Get()
{
    if (!m_managerInstance)
        m_managerInstance = new AssetManager();

    // static AssetManager m_managerInstance;
    return m_managerInstance;
}

void AssetManager::Init(Device* device) { m_device = device; }

void AssetManager::Shutdown() { m_assets.clear(); }

void AssetManager::LoadAsset(const std::string& filename) { assert(!filename.empty()); }

void AssetManager::RegisterAsset(const std::string& name, std::shared_ptr<Asset> asset)
{
    assert(asset);
    assert(!name.empty());
    if (auto it = m_assets.find(name); it != m_assets.end())
    {
        if (it->second)
        {
            printf("Asset trying to be loaded already found in AssetManager!\n");
            return;
        }
    }

    m_assets.insert({name, asset});
}

void AssetManager::ReleaseAsset(const std::string& name)
{
    assert(!name.empty());
    const auto it = m_assets.find(name);
    if (it == m_assets.end())
    {
        printf("[WARN]: Trying to release asset not registered in the AssetManager.");
        return;
    }

    it->second.reset();
}
} // namespace Pinut
