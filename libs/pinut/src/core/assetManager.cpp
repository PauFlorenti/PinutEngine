#include "stdafx.h"

#include "assetManager.h"
#include "src/assets/asset.h"

namespace Pinut
{

AssetManager& AssetManager::Get()
{
    static AssetManager m_instance;
    return m_instance;
}

void AssetManager::Init() {}

void AssetManager::Shutdown() { m_assets.clear(); }

void AssetManager::LoadAsset(const std::string& filename) { assert(!filename.empty()); }

void AssetManager::RegisterAsset(const std::string& name, Asset* asset)
{
    assert(asset);
    assert(!name.empty());
    if (m_assets.find(name) != m_assets.end())
        printf("Asset trying to be loaded already found in AssetManager!\n");
    else
    {
        m_assets.insert({name, {std::move(asset), 1}});
    }
}

Asset* AssetManager::GetAsset(const std::string& name)
{
    assert(!name.empty());
    const auto it = m_assets.find(name);

    if (it != m_assets.end())
    {
        it->second.second++;
        return it->second.first;
    }

    printf("[ERROR]: Asset not registered in AssetManager. Could not retrieve it.");
    return nullptr;
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

    if (it->second.second > 0)
    {
        it->second.second--;
    }
    else
    {
        m_assets.erase(it);
    }
}

} // namespace Pinut
