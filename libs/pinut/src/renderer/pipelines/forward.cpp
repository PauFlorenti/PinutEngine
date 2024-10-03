//#include "stdafx.h"
//
//#include "forward.h"
//#include "src/assets/mesh.h"
//#include "src/assets/texture.h"
//#include "src/core/assetManager.h"
//#include "src/core/camera.h"
//#include "src/core/node.h"
//#include "src/core/renderable.h"
//#include "src/core/scene.h"
//#include "src/renderer/common.h"
//#include "src/renderer/device.h"
//#include "src/renderer/pipeline.h"
//#include "src/renderer/primitives.h"
//#include "src/renderer/utils.h"
//
//namespace Pinut
//{
//void ForwardPipeline::Init(Device* device)
//{
//    assert(device);
//    m_device = device;
//
//    auto logicalDevice = m_device->GetDevice();
//
//    std::vector<VkDescriptorPoolSize> sizes = {
//      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
//    };
//
//    m_descriptorSetManager.OnCreate(m_device->GetDevice(), 3, 10, std::move(sizes));
//
//    m_perFrameBuffer.Create(m_device, sizeof(PerFrameData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
//
//    m_perObjectBuffer.Create(m_device, sizeof(u32), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
//
//    m_transformsBuffer.Create(m_device,
//                              sizeof(glm::mat4) * MAX_ENTITIES,
//                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
//
//    m_lightsBuffer.Create(m_device, sizeof(SceneLightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
//
//    m_opaqueStage.BuildPipeline(logicalDevice);
//    m_skyboxStage.BuildPipeline(logicalDevice);
//    m_transparentStage.BuildPipeline(logicalDevice);
//
//#ifdef _DEBUG
//    m_wireframeStage.BuildPipeline(logicalDevice);
//#endif
//}
//
//void ForwardPipeline::Shutdown()
//{
//    const auto device = m_device->GetDevice();
//
//    OnDestroyWindowDependantResources();
//
//    m_descriptorSetManager.OnDestroy();
//
//    m_lightsBuffer.Destroy();
//    m_transformsBuffer.Destroy();
//    m_perObjectBuffer.Destroy();
//    m_perFrameBuffer.Destroy();
//
//    m_opaqueStage.Destroy(device);
//    m_skyboxStage.Destroy(device);
//    m_transparentStage.Destroy(device);
//#ifdef _DEBUG
//    m_wireframeStage.Destroy(device);
//#endif
//}
//
//void ForwardPipeline::OnCreateWindowDependantResources(u32 width, u32 height)
//{
//    VkImageCreateInfo depthTextureInfo{
//      .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
//      .pNext                 = nullptr,
//      .imageType             = VK_IMAGE_TYPE_2D,
//      .format                = VK_FORMAT_D32_SFLOAT,
//      .extent                = {width, height, 1},
//      .mipLevels             = 1,
//      .arrayLayers           = 1,
//      .samples               = VK_SAMPLE_COUNT_1_BIT,
//      .tiling                = VK_IMAGE_TILING_OPTIMAL,
//      .usage                 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
//      .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
//      .queueFamilyIndexCount = 0,
//      .pQueueFamilyIndices   = nullptr,
//      .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
//    };
//
//    m_depthTexture = std::make_shared<Texture>(m_device, depthTextureInfo);
//}
//
//void ForwardPipeline::OnDestroyWindowDependantResources()
//{
//    m_device->WaitIdle();
//
//    if (m_depthTexture != nullptr)
//    {
//        m_depthTexture->Destroy();
//        m_depthTexture = nullptr;
//    }
//}
//
//void ForwardPipeline::Render(VkCommandBuffer cmd, Camera* camera, Scene* scene)
//{
//    assert(camera);
//    assert(scene);
//
//    const auto  device                 = m_device->GetDevice();
//    const auto& transparentRenderables = scene->TransparentRenderables();
//
//    m_descriptorSetManager.Clear();
//
//    // Camera
//    {
//        auto data = (PerFrameData*)m_perFrameBuffer.AllocationInfo().pMappedData;
//
//        data->view           = camera->View();
//        data->projection     = camera->Projection();
//        data->cameraPosition = camera->Position();
//    }
//
//    // Lights
//    {
//        auto lightData         = (SceneLightData*)m_lightsBuffer.AllocationInfo().pMappedData;
//        lightData->lightsCount = scene->LightsCount();
//
//        u32 index = 0;
//        for (const auto& l : scene->Lights())
//        {
//            if (!l)
//                continue;
//
//            auto& data     = lightData->lights[index];
//            data.color     = l->m_color;
//            data.intensity = l->m_enabled ? l->m_intensity : 0.0f;
//            data.position  = l->GetPosition();
//            data.direction = l->GetDirection();
//
//            if (const auto& point = std::dynamic_pointer_cast<PointLight>(l))
//            {
//                data.radius = point->m_radius;
//            }
//            if (const auto& spot = std::dynamic_pointer_cast<SpotLight>(l))
//            {
//                data.innerCone      = glm::radians(static_cast<f32>(spot->m_innerCone));
//                data.outerCone      = glm::radians(static_cast<f32>(spot->m_outerCone));
//                data.cosineExponent = spot->m_cosineExponent;
//            }
//
//            index++;
//        }
//
//        const auto& directionalLight          = scene->GetDirectionalLight();
//        lightData->directionalLight.direction = directionalLight.GetDirection();
//        lightData->directionalLight.color     = directionalLight.m_color;
//        lightData->directionalLight.intensity =
//          directionalLight.m_enabled ? directionalLight.m_intensity : 0.0f;
//    }
//
//    // Transforms
//    u32 i = 0;
//    for (const auto& r : scene->Renderables())
//    {
//        if (i == MAX_ENTITIES)
//            break;
//
//        auto transforms = (glm::mat4*)m_transformsBuffer.AllocationInfo().pMappedData;
//        for (auto& node : r->GetAllNodes())
//        {
//            transforms[i] = node->GetGlobalMatrix();
//            node->SetInstanceIndex(i);
//            ++i;
//        }
//    }
//
//    DrawOpaque(cmd, scene);
//
//    // Draw skybox
//    DrawSkybox(cmd, camera);
//
//    if (!transparentRenderables.empty())
//        DrawTransparents(cmd, scene);
//
//#ifdef _DEBUG
//    DrawDebug(cmd, scene);
//#endif
//}
//
//void ForwardPipeline::DrawOpaque(VkCommandBuffer cmd, Scene* scene)
//{
//    m_opaqueStage.BindPipeline(cmd);
//
//    const auto set = m_descriptorSetManager.Allocate(m_opaqueStage.m_perFrameDescriptorSetLayout);
//    vkCmdBindDescriptorSets(cmd,
//                            VK_PIPELINE_BIND_POINT_GRAPHICS,
//                            m_opaqueStage.m_pipelineLayout,
//                            0,
//                            1,
//                            &set,
//                            0,
//                            nullptr);
//
//    auto bufferVertexInfo =
//      vkinit::DescriptorBufferInfo(m_perFrameBuffer.m_buffer, 0, sizeof(PerFrameData));
//    auto transformBufferInfo = vkinit::DescriptorBufferInfo(m_transformsBuffer.m_buffer,
//                                                            0,
//                                                            sizeof(glm::mat4) * MAX_ENTITIES);
//    auto lightsBufferInfo = vkinit::DescriptorBufferInfo(m_lightsBuffer.m_buffer, 0, VK_WHOLE_SIZE);
//    VkWriteDescriptorSet perFrameWrites[3] = {
//      vkinit::WriteDescriptorSet(set, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferVertexInfo),
//      vkinit::WriteDescriptorSet(set,
//                                 1,
//                                 1,
//                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//                                 &transformBufferInfo),
//      vkinit::WriteDescriptorSet(set, 2, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &lightsBufferInfo),
//    };
//    vkUpdateDescriptorSets(m_device->GetDevice(), 3, perFrameWrites, 0, nullptr);
//
//    for (const auto& renderable : scene->Renderables())
//    {
//        renderable->Draw(cmd, m_opaqueStage.m_pipelineLayout);
//    }
//}
//
//void ForwardPipeline::DrawSkybox(VkCommandBuffer cmd, Camera* camera)
//{
//    m_skyboxStage.BindPipeline(cmd);
//    const auto sphereMesh = m_assetManager.GetAsset<Mesh>("sphere.obj");
//
//    const auto model = glm::translate(glm::mat4(1.0f), camera->Position());
//    vkCmdPushConstants(cmd,
//                       m_skyboxStage.m_pipelineLayout,
//                       VK_SHADER_STAGE_VERTEX_BIT,
//                       0,
//                       sizeof(glm::mat4),
//                       &model);
//
//    std::array<VkDescriptorSet, 2> set = {
//      m_descriptorSetManager.Allocate(m_skyboxStage.m_perFrameDescriptorSetLayout),
//      m_descriptorSetManager.Allocate(m_skyboxStage.m_perObjectDescriptorSetLayout),
//    };
//
//    vkCmdBindDescriptorSets(cmd,
//                            VK_PIPELINE_BIND_POINT_GRAPHICS,
//                            m_skyboxStage.m_pipelineLayout,
//                            0,
//                            2,
//                            set.data(),
//                            0,
//                            nullptr);
//
//    // TODO This should be provided by the scene ...
//    const auto t         = m_assetManager.GetAsset<Texture>("woods.jpg");
//    auto       imageInfo = vkinit::DescriptorImageInfo(t->ImageView(),
//                                                 t->Sampler(),
//                                                 VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//
//    auto bufferVertexInfo =
//      vkinit::DescriptorBufferInfo(m_perFrameBuffer.m_buffer, 0, sizeof(PerFrameData));
//
//    VkWriteDescriptorSet writes[2] = {
//      vkinit::WriteDescriptorSet(set.at(0),
//                                 0,
//                                 1,
//                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                                 &bufferVertexInfo),
//      vkinit::WriteDescriptorSet(set.at(1),
//                                 0,
//                                 1,
//                                 VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//                                 nullptr,
//                                 &imageInfo),
//    };
//
//    vkUpdateDescriptorSets(m_device->GetDevice(), 2, writes, 0, nullptr);
//
//    sphereMesh->Draw(cmd, m_skyboxStage.m_pipelineLayout);
//}
//
//void ForwardPipeline::DrawTransparents(VkCommandBuffer cmd, Scene* scene)
//{
//    m_transparentStage.BindPipeline(cmd);
//    const auto transparentSet =
//      m_descriptorSetManager.Allocate(m_transparentStage.m_perFrameDescriptorSetLayout);
//
//    vkCmdBindDescriptorSets(cmd,
//                            VK_PIPELINE_BIND_POINT_GRAPHICS,
//                            m_transparentStage.m_pipelineLayout,
//                            0,
//                            1,
//                            &transparentSet,
//                            0,
//                            nullptr);
//
//    auto bufferVertexInfo =
//      vkinit::DescriptorBufferInfo(m_perFrameBuffer.m_buffer, 0, sizeof(PerFrameData));
//    auto transformBufferInfo = vkinit::DescriptorBufferInfo(m_transformsBuffer.m_buffer,
//                                                            0,
//                                                            sizeof(glm::mat4) * MAX_ENTITIES);
//
//    VkWriteDescriptorSet writes[2] = {vkinit::WriteDescriptorSet(transparentSet,
//                                                                 0,
//                                                                 1,
//                                                                 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//                                                                 &bufferVertexInfo),
//                                      vkinit::WriteDescriptorSet(transparentSet,
//                                                                 1,
//                                                                 1,
//                                                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//                                                                 &transformBufferInfo)};
//    vkUpdateDescriptorSets(m_device->GetDevice(), 2, std::move(writes), 0, nullptr);
//
//    std::shared_ptr<MaterialInstance> currentMaterial{nullptr};
//
//    //for (auto& dc : scene->TransparentRenderables())
//    //{
//    //    if (!currentMaterial || currentMaterial != dc.m_material)
//    //    {
//    //        currentMaterial = dc.m_material;
//    //        currentMaterial->Bind(cmd);
//    //    }
//
//    //    dc.Draw(cmd);
//    //}
//}
//
//void ForwardPipeline::DrawDebug(VkCommandBuffer cmd, Scene* scene)
//{
//    m_wireframeStage.BindPipeline(cmd);
//
//    const auto set =
//      m_descriptorSetManager.Allocate(m_wireframeStage.m_perFrameDescriptorSetLayout);
//
//    vkCmdBindDescriptorSets(cmd,
//                            VK_PIPELINE_BIND_POINT_GRAPHICS,
//                            m_wireframeStage.m_pipelineLayout,
//                            0,
//                            1,
//                            &set,
//                            0,
//                            nullptr);
//
//    auto bufferVertexInfo =
//      vkinit::DescriptorBufferInfo(m_perFrameBuffer.m_buffer, 0, sizeof(PerFrameData));
//
//    auto write =
//      vkinit::WriteDescriptorSet(set, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferVertexInfo);
//    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &write, 0, nullptr);
//
//    u32 i = 0;
//    for (const auto& l : scene->Lights())
//    {
//        if (i == scene->LightsCount())
//            break;
//
//        if (!l->m_renderDebug)
//        {
//            ++i;
//            continue;
//        }
//
//        if (const auto& spot = std::dynamic_pointer_cast<SpotLight>(l))
//        {
//            const auto position  = spot->GetPosition();
//            const auto direction = spot->GetDirection();
//            const auto radius    = spot->m_radius;
//
//            const auto ComputeBaseConeRadius = [&position, &direction, &radius](f32 angle)
//            {
//                const auto end      = position + direction * radius;
//                const auto distance = glm::length(end - position);
//                const auto h        = distance / glm::cos(glm::radians(angle));
//                return glm::sqrt(h * h - distance * distance);
//            };
//
//            Primitives::DrawWiredCone(cmd,
//                                      m_wireframeStage.m_pipelineLayout,
//                                      position,
//                                      position + direction * radius,
//                                      ComputeBaseConeRadius(spot->m_outerCone));
//
//            Primitives::DrawWiredCone(cmd,
//                                      m_wireframeStage.m_pipelineLayout,
//                                      position,
//                                      position + direction * radius,
//                                      ComputeBaseConeRadius(spot->m_innerCone),
//                                      glm::vec3(0.0f, 1.0f, 1.0f));
//        }
//        else if (const auto& point = std::dynamic_pointer_cast<PointLight>(l))
//        {
//            Primitives::DrawWiredSphere(cmd,
//                                        m_wireframeStage.m_pipelineLayout,
//                                        glm::translate(glm::mat4(1.0f), point->GetPosition()),
//                                        point->m_radius);
//        }
//
//        ++i;
//    }
//}
//} // namespace Pinut
