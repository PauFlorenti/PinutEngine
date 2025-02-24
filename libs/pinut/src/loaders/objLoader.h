#pragma once

namespace Pinut
{
class AssetManager;
class Mesh;
class Renderable;
class OBJLoader final
{
  public:
    OBJLoader();

    // TODO Loader should not create renderables ??
    // TODO It probably should not receive an assetManager either.
    std::shared_ptr<Renderable> LoadRenderableFromFile(const std::filesystem::path& filepath,
                                                       AssetManager&                assetManager);
    std::shared_ptr<Mesh>       LoadMeshFromFile(const std::filesystem::path& filepath,
                                                 AssetManager&                assetManager);
};
} // namespace Pinut
