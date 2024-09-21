#pragma once

namespace tinygltf
{
class Model;
class Node;
} // namespace tinygltf
namespace Pinut
{
class AssetManager;
class Device;
class Node;
class Renderable;
class GLTFLoader final
{
  public:
    GLTFLoader();
    void Init(Device* device, VkDescriptorSetLayout layout);

    std::shared_ptr<Renderable> LoadFromFile(const std::filesystem::path& filepath,
                                             AssetManager&                assetManager);

  private:
    std::shared_ptr<Node> LoadNode(const tinygltf::Model& tmodel,
                                   const tinygltf::Node&  tnode,
                                   std::shared_ptr<Node>  parent,
                                   AssetManager&          assetManager,
                                   const bool             invertNormals = false);

    Device* m_device{nullptr};
    VkDescriptorSetLayout m_layout; // TODO This is temporarily hardcoded
};
} // namespace Pinut
