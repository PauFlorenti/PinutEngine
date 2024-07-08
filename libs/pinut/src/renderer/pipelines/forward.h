#pragma once

#include <vulkan/vulkan.h>

#include "src/renderer/buffer.h"
#include "src/renderer/descriptorSetManager.h"

namespace Pinut
{
class Camera;
class Device;
class Scene;
class Texture;
struct PerFrameData;
class ForwardPipeline
{
  public:
    void Init(Device* device);
    void Shutdown();

    void OnDestroyWindowDependantResources();
    void OnCreateWindowDependantResources(u32 width, u32 height);

    const Texture* GetDepthAttachment() const { return m_depthTexture; }

    void BindPipeline(VkCommandBuffer cmd);
    void Render(VkCommandBuffer cmd, Camera* camera, Scene* scene);

    VkPipeline Pipeline() const;

  private:
    Device*              m_device{nullptr};
    VkPipeline           m_pipeline{VK_NULL_HANDLE};
    VkPipelineLayout     m_pipelineLayout{VK_NULL_HANDLE};
    DescriptorSetManager m_descriptorSetManager;

    VkDescriptorSetLayout m_perFrameDescriptorSetLayout{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_perObjectDescriptorSetLayout{VK_NULL_HANDLE};

    GPUBuffer m_perFrameBuffer;
    GPUBuffer m_perObjectBuffer;
    Texture*  m_depthTexture{nullptr};
};
} // namespace Pinut
