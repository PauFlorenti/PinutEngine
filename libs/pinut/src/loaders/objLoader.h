#pragma once

namespace RED
{
class Device;
}
namespace Pinut
{
class AssetManager;
class Mesh;
class Renderable;
class OBJLoader final
{
  public:
    OBJLoader();
    void Init(std::weak_ptr<RED::Device> device);

    // TODO Loader should not create renderables ??
    // TODO It probably should not receive an assetManager either.
    std::shared_ptr<Renderable> LoadRenderableFromFile(const std::filesystem::path& filepath,
                                                       AssetManager&                assetManager);
    std::shared_ptr<Mesh>       LoadMeshFromFile(const std::filesystem::path& filepath,
                                                 AssetManager&                assetManager);

  private:
    std::weak_ptr<RED::Device> m_device;
};
} // namespace Pinut
