#include "stdafx.h"

#include "forward.h"
#include "src/assets/mesh.h"
#include "src/assets/texture.h"
#include "src/core/camera.h"
#include "src/core/scene.h"
#include "src/renderer/common.h"
#include "src/renderer/device.h"
#include "src/renderer/materials/material.h"
#include "src/renderer/materials/opaqueMaterial.h"
#include "src/renderer/pipeline.h"
#include "src/renderer/renderable.h"
#include "src/renderer/utils.h"

namespace Pinut
{
void ForwardPipeline::Init(Device* device)
{
    assert(device);
    m_device = device;

    auto logicalDevice = m_device->GetDevice();

    std::vector<VkDescriptorPoolSize> sizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
    };

    m_descriptorSetManager.OnCreate(m_device->GetDevice(), 3, 1, std::move(sizes));

    m_perFrameBuffer.Create(m_device, sizeof(PerFrameData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    m_perObjectBuffer.Create(m_device, sizeof(u32), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

    m_transformsBuffer.Create(m_device,
                              sizeof(glm::mat4) * MAX_ENTITIES,
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

    m_lightsBuffer.Create(m_device, sizeof(SceneLightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
}

void ForwardPipeline::Shutdown()
{
    const auto device = m_device->GetDevice();

    OnDestroyWindowDependantResources();

    m_descriptorSetManager.OnDestroy();

    m_lightsBuffer.Destroy();
    m_transformsBuffer.Destroy();
    m_perObjectBuffer.Destroy();
    m_perFrameBuffer.Destroy();
}

void ForwardPipeline::OnCreateWindowDependantResources(u32 width, u32 height)
{
    VkImageCreateInfo depthTextureInfo{
      .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext                 = nullptr,
      .imageType             = VK_IMAGE_TYPE_2D,
      .format                = VK_FORMAT_D32_SFLOAT,
      .extent                = {width, height, 1},
      .mipLevels             = 1,
      .arrayLayers           = 1,
      .samples               = VK_SAMPLE_COUNT_1_BIT,
      .tiling                = VK_IMAGE_TILING_OPTIMAL,
      .usage                 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices   = nullptr,
      .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    m_depthTexture = std::make_shared<Texture>(m_device, depthTextureInfo);
}

void ForwardPipeline::OnDestroyWindowDependantResources()
{
    m_device->WaitIdle();

    if (m_depthTexture != nullptr)
    {
        m_depthTexture->Destroy();
        m_depthTexture = nullptr;
    }
}

void ForwardPipeline::Render(VkCommandBuffer cmd, Camera* camera, Scene* scene)
{
    assert(camera);
    assert(scene);

    const auto  device                 = m_device->GetDevice();
    const auto& renderables            = scene->OpaqueRenderables();
    const auto& transparentRenderables = scene->TransparentRenderables();

    assert(!renderables.empty());

    const auto opaqueMaterialInstance = renderables.at(0).m_material;

    m_descriptorSetManager.Clear();
    opaqueMaterialInstance->BindPipeline(cmd);

    // Camera
    {
        auto data = (PerFrameData*)m_perFrameBuffer.AllocationInfo().pMappedData;

        data->view           = camera->View();
        data->projection     = camera->Projection();
        data->cameraPosition = camera->Position();
    }

    // Lights
    {
        auto lightData         = (SceneLightData*)m_lightsBuffer.AllocationInfo().pMappedData;
        lightData->lightsCount = scene->LightsCount();

        u32 index = 0;
        for (const auto& l : scene->Lights())
        {
            auto& data          = lightData->lights[index];
            data.color          = l.color;
            data.intensity      = l.intensity;
            data.position       = l.position;
            data.radius         = l.radius;
            data.cosine         = glm::cos(glm::radians(l.cosine));
            data.cosineExponent = l.cosineExponent;
            data.direction =
              data.cosine > 0.0f ? l.rotation * glm::vec3(0.0f, 0.0f, -1.0f) : glm::vec3{};

            index++;
        }

        //memset(lightData + sizeof(LightData) * index, 0, sizeof(lightData) * (MAX_LIGHTS - index));

        const auto& directionalLight = scene->GetDirectionalLight();
        lightData->directionalLight.direction =
          directionalLight.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        lightData->directionalLight.color     = directionalLight.color;
        lightData->directionalLight.intensity = directionalLight.intensity;
    }

    // Transforms
    u32 i = 0;
    for (const auto& r : scene->Renderables())
    {
        auto transforms = (glm::mat4*)m_transformsBuffer.AllocationInfo().pMappedData;
        transforms[i]   = r->Model();
        r->SetInstanceIndex(i);
        ++i;
    }

    auto opaqueMaterial = (OpaqueMaterial*)opaqueMaterialInstance->GetMaterial();
    auto set = m_descriptorSetManager.Allocate(opaqueMaterial->m_perFrameDescriptorSetLayout);
    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            opaqueMaterial->m_pipelineLayout,
                            0,
                            1,
                            &set,
                            0,
                            nullptr);

    auto bufferVertexInfo =
      vkinit::DescriptorBufferInfo(m_perFrameBuffer.m_buffer, 0, sizeof(PerFrameData));
    auto transformBufferInfo = vkinit::DescriptorBufferInfo(m_transformsBuffer.m_buffer,
                                                            0,
                                                            sizeof(glm::mat4) * MAX_ENTITIES);
    auto lightsBufferInfo = vkinit::DescriptorBufferInfo(m_lightsBuffer.m_buffer, 0, VK_WHOLE_SIZE);
    VkWriteDescriptorSet perFrameWrites[3] = {
      vkinit::WriteDescriptorSet(set, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferVertexInfo),
      vkinit::WriteDescriptorSet(set,
                                 1,
                                 1,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                 &transformBufferInfo),
      vkinit::WriteDescriptorSet(set, 2, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &lightsBufferInfo),
    };
    vkUpdateDescriptorSets(m_device->GetDevice(), 3, perFrameWrites, 0, nullptr);

    // Draw

    std::shared_ptr<MaterialInstance> currentMaterial{nullptr};
    for (const auto& dc : scene->OpaqueRenderables())
    {
        if (!currentMaterial || *currentMaterial != *dc.m_material)
        {
            currentMaterial = dc.m_material;
            currentMaterial->Bind(cmd);
        }

        dc.Draw(cmd);
    }

    if (transparentRenderables.empty())
        return;

    transparentRenderables.at(0).m_material->BindPipeline(cmd);
    for (auto& dc : scene->TransparentRenderables())
    {
        if (currentMaterial != dc.m_material)
        {
            currentMaterial = dc.m_material;
            currentMaterial->Bind(cmd);
        }

        dc.Draw(cmd);
    }
}
} // namespace Pinut
