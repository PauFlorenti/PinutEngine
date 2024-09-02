#pragma once

#include <vulkan/vulkan.h>

#include "src/renderer/buffer.h"
#include "src/renderer/descriptorSetManager.h"
#include "src/renderer/materials/skyboxMaterial.h"

namespace Pinut
{
class AssetManager;
class Camera;
class DescriptorSetManager;
class Device;
class Scene;
class Texture;
struct PerFrameData;
class ForwardPipeline
{
  public:
    ForwardPipeline(AssetManager& assetManager) : m_assetManager(assetManager){}
    void Init(Device* device);
    void Shutdown();

    void OnDestroyWindowDependantResources();
    void OnCreateWindowDependantResources(u32 width, u32 height);

    const std::shared_ptr<Texture>& GetDepthAttachment() const { return m_depthTexture; }

    void Render(VkCommandBuffer cmd, Camera* camera, Scene* scene);

  private:
    void DrawSkybox(VkCommandBuffer cmd, Camera* camera);

    Device*               m_device{nullptr};
    VkDescriptorSetLayout m_transformsDescriptorSetLayout{VK_NULL_HANDLE};

    DescriptorSetManager m_descriptorSetManager; // Used for global data.
    AssetManager&        m_assetManager;

    std::shared_ptr<SkyboxMaterial> m_skyboxMaterial{nullptr};

    GPUBuffer                m_perFrameBuffer;
    GPUBuffer                m_perObjectBuffer;
    GPUBuffer                m_lightsBuffer;
    GPUBuffer                m_transformsBuffer;
    std::shared_ptr<Texture> m_depthTexture{nullptr};
};
} // namespace Pinut
