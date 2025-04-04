#include "pch.hpp"

#define GLFW_INCLUDE_VULKAN
#include <glfw/include/GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <tinygltf/json.hpp>

#include <render_device/bufferDescriptor.h>
#include <render_device/device.h>
#include <render_device/drawCall.h>
#include <render_device/renderPipeline.h>
#include <render_device/shader.h>
#include <render_device/textureDescriptor.h>
#include <render_device/textureFormat.h>

#include "pinut/assets/material.h"
#include "pinut/assets/mesh.h"
#include "pinut/components/lightComponent.h"
#include "pinut/components/meshComponent.h"
#include "pinut/components/renderComponent.h"
#include "pinut/components/skyComponent.h"
#include "pinut/components/transformComponent.h"
#include "pinut/imgui/pinutImgui.h"
#include "pinut/renderer/materialData.h"
#include "pinut/renderer/meshData.h"
#include "pinut/renderer/renderer.h"
#include "pinut/renderer/skyData.h"
#include "pinut/renderer/swapchain.h"
#include "pinut/renderer/utils.h"

namespace Pinut
{
std::unordered_map<std::string, RED::RenderPipeline> m_pipelines;

RED::Shader PopulateShaderFromJson(const nlohmann::json& j, RED::ShaderType shaderType)
{
    RED::Shader shader{j["name"].get<std::string>(), shaderType};

    if (j.contains("uniforms"))
    {
        const auto& jUniforms = j["uniforms"];
        shader.uniformDataSlots.reserve(jUniforms.size());
        for (const auto& uniform : jUniforms)
        {
            const auto set  = uniform["set"].get<u32>();
            const auto type = uniform["type"].get<std::string>();
            shader.uniformDataSlots.emplace_back(shaderType,
                                                 type == "Buffer" ? RED::UniformType::BUFFER :
                                                                    RED::UniformType::TEXTURE,
                                                 uniform["name"].get<std::string>(),
                                                 uniform["binding"].get<i32>(),
                                                 uniform.value("count", 1),
                                                 set);

            if (std::find(shader.descriptorSets.begin(), shader.descriptorSets.end(), set) ==
                shader.descriptorSets.end())
            {
                shader.descriptorSets.emplace_back(set);
            }
        }
    }

    return shader;
}

std::vector<RED::TextureFormat> PopulateAttachmentsFromJson(const nlohmann::json& j)
{
    if (!j.contains("attachments"))
        return {};

    const auto                      jAttachments = j["attachments"];
    std::vector<RED::TextureFormat> formats;
    formats.reserve(jAttachments.size());

    for (const auto& jAttachment : jAttachments)
    {
        const auto attachment = jAttachment.get<std::string>();
        formats.emplace_back(GetFormatFromString(attachment));
    }

    return formats;
}

Renderer::Renderer(std::shared_ptr<RED::Device> device,
                   SwapchainInfo*               swapchain,
                   std::unique_ptr<PinutImGUI>  imgui)
: m_device(device),
  m_swapchain(swapchain),
#ifdef _DEBUG
  m_imgui(std::move(imgui))
#endif
{
    // Init pipelines
    std::ifstream pipelineFile(std::filesystem::path("../libs/pinut/pipelines/pipelines.json"));

    if (!pipelineFile.is_open())
    {
        printf("[ERROR]: Could not open pipelines.json file.");
        return;
    }

    nlohmann::json jPipelineFile;
    try
    {
        jPipelineFile = nlohmann::json::parse(pipelineFile);
    }
    catch (nlohmann::json::parse_error&)
    {
        pipelineFile.close();
        printf("[ERROR]: Could not parse pipelines.json file.");
        return;
    }

    for (auto& j : jPipelineFile.items())
    {
        auto jdata = j.value();

        m_pipelines.insert({j.key(),
                            {j.key().c_str(),
                             PopulateShaderFromJson(jdata["vs"], RED::ShaderType::VERTEX),
                             PopulateShaderFromJson(jdata["fs"], RED::ShaderType::FRAGMENT),
                             jdata.value("input_vertex", "Pos"),
                             PopulateAttachmentsFromJson(jdata),
                             GetFormatFromString(jdata.value("depth", ""))}});
    }
}

Renderer::~Renderer()
{
    m_device->WaitIdle();
#ifdef _DEBUG
    m_imgui.reset();
#endif
    m_rendererRegistry.clear();
    m_rendererRegistry.ctx().erase<SkyData>();
    m_offscreenState.Clear();
    m_device.reset();
}

void Renderer::Update(entt::registry& registry, const ViewportData& viewportData, bool resized)
{
    if (resized)
    {
        m_offscreenState.Clear();
        m_offscreenState.Create(*m_device,
                                viewportData.width,
                                viewportData.height,
                                {RED::TextureFormat::R32G32B32A32_SFLOAT},
                                true,
                                RED::TextureFormat::D32_SFLOAT);
    }

    std::array<LightData, 4> lightData;
    memset(lightData.data(), 0, sizeof(LightData) * lightData.size());
    u32 lightIndex{0};
    registry.view<Component::LightComponent, Component::TransformComponent>().each(
      [this, &lightData, &lightIndex](entt::entity entity,
                                      auto&        lightComponent,
                                      auto&        transformComponent)
      {
          if (!lightComponent.m_enabled || lightComponent.m_intensity <= 0.0f)
              return;

          lightData.at(lightIndex).color          = lightComponent.m_color;
          lightData.at(lightIndex).intensity      = lightComponent.m_intensity;
          lightData.at(lightIndex).position       = transformComponent.GetPosition();
          lightData.at(lightIndex).radius         = lightComponent.m_radius;
          lightData.at(lightIndex).direction      = transformComponent.GetForward();
          lightData.at(lightIndex).innerCone      = lightComponent.m_innerCone;
          lightData.at(lightIndex).outerCone      = lightComponent.m_outerCone;
          lightData.at(lightIndex).cosineExponent = lightComponent.m_cosineExponent;

          ++lightIndex;
      });
    m_device->UpdateBuffer(m_offscreenState.lightsBuffer.GetID(), lightData.data());

    registry.view<Component::RenderComponent, Component::MeshComponent>().each(
      [this](entt::entity entity, auto& renderComponent, auto& meshComponent)
      {
          CreateMeshData(m_device, m_rendererRegistry, meshComponent.m_mesh);

          if (!m_rendererRegistry.try_get<MaterialData>(renderComponent.material->m_handle))
          {
              renderComponent.material->m_handle = m_rendererRegistry.create();
              auto& materialData =
                m_rendererRegistry.emplace<MaterialData>(renderComponent.material->m_handle);

              auto CreateMaterialDataTexture =
                [&device = this->m_device](std::shared_ptr<Texture> t)
              {
                  RED::TextureDescriptor textureDescriptor{t->GetWidth(),
                                                           t->GetHeight(),
                                                           1,
                                                           t->GetFormat(),
                                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                           VK_IMAGE_USAGE_SAMPLED_BIT};
                  return device->CreateTexture(textureDescriptor, t->GetData());
              };

              auto material = renderComponent.material;

              materialData.difuseTexture = CreateMaterialDataTexture(material->diffuseTexture);
              materialData.normalTexture = CreateMaterialDataTexture(material->normalTexture);
              materialData.metallicRoughnessTexture =
                CreateMaterialDataTexture(material->metallicRoughnessTexture);
              materialData.emissiveTexture = CreateMaterialDataTexture(material->emissiveTexture);

              materialData.modelBuffer =
                m_device->CreateBuffer({64, 64, RED::BufferUsage::UNIFORM});

              u32 uniformMaterialData[4] = {material->m_diffuse.RGBA(),
                                            material->m_specular.RGBA(),
                                            material->m_emissive.RGBA(),
                                            0};
              materialData.uniformBuffer =
                m_device->CreateBuffer({16, 16, RED::BufferUsage::UNIFORM}, &uniformMaterialData);
          }
      });

    if (auto skyEntt = registry.view<Component::SkyComponent>().front(); skyEntt != entt::null)
    {
        auto& skyComponent = registry.get<Component::SkyComponent>(skyEntt);

        if (skyComponent.m_dirty)
        {
            CreateMeshData(m_device, m_rendererRegistry, skyComponent.GetMesh());

            auto& skyData      = m_rendererRegistry.ctx().emplace<SkyData>();
            skyData.meshHandle = skyComponent.GetMesh()->m_handle;

            const auto&            texture = skyComponent.GetTexture();
            RED::TextureDescriptor textureDescriptor{texture->GetWidth(),
                                                     texture->GetHeight(),
                                                     1,
                                                     texture->GetFormat(),
                                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                     VK_IMAGE_USAGE_SAMPLED_BIT};
            skyData.skyTexture =
              m_device->CreateTexture(std::move(textureDescriptor), texture->GetData());

            skyData.skyMeshDataBuffer = m_device->CreateBuffer(
              {sizeof(SkyboxUniformData), sizeof(SkyboxUniformData), RED::BufferUsage::UNIFORM});

            skyComponent.m_dirty = false;
        }
    }
}

void Renderer::Render(entt::registry& registry, const ViewportData& viewportData, bool resized)
{
    m_device->BeginFrame();

    Update(registry, viewportData, resized);

    // PASS
    // Start updating/uploading data to gpu resources.
    // Cannot be done inside a render pass.
    auto cameraData = viewportData.cameraData;
    m_device->UpdateBuffer(m_offscreenState.globalUniformBuffer.GetID(), (void*)&cameraData);

    std::vector<RED::DrawCall> drawCalls;
    drawCalls.reserve(1000);
    std::vector<RED::DrawCall> depthPassDrawCalls;
    depthPassDrawCalls.reserve(1000);

    registry
      .view<Component::TransformComponent, Component::MeshComponent, Component::RenderComponent>()
      .each(
        [&drawCalls,
         &depthPassDrawCalls,
         this](auto entity, auto& transformComponent, auto& meshComponent, auto& renderComponent)
        {
            const auto meshData =
              m_rendererRegistry.try_get<MeshData>(meshComponent.m_mesh->m_handle);
            const auto materialData =
              m_rendererRegistry.try_get<MaterialData>(renderComponent.material->m_handle);

            if (!meshData || !materialData)
                return;

            auto transform = transformComponent.GetTransform();
            m_device->UpdateBuffer(materialData->modelBuffer.GetID(), &transform);

            RED::DrawCall dc;
            RED::DrawCall depthDrawCall;
            dc.vertexBuffer = depthDrawCall.vertexBuffer = meshData->m_vertexBuffer;
            dc.indexBuffer = depthDrawCall.indexBuffer = meshData->m_indexBuffer;

            dc.SetUniformBuffer({m_offscreenState.globalUniformBuffer}, RED::ShaderType::VERTEX, 0);
            dc.SetUniformBuffer({m_offscreenState.lightsBuffer}, RED::ShaderType::FRAGMENT, 1);
            dc.SetUniformBuffer({materialData->modelBuffer}, RED::ShaderType::VERTEX, 0, 1);
            dc.SetUniformBuffer({materialData->uniformBuffer}, RED::ShaderType::FRAGMENT, 1, 1);
            dc.SetUniformTexture({materialData->difuseTexture,
                                  materialData->normalTexture,
                                  materialData->metallicRoughnessTexture,
                                  materialData->emissiveTexture,
                                  materialData->emissiveTexture},
                                 RED::ShaderType::FRAGMENT,
                                 2,
                                 1);

            depthDrawCall.SetUniformBuffer({m_offscreenState.globalUniformBuffer},
                                           RED::ShaderType::VERTEX,
                                           0,
                                           0);
            depthDrawCall.SetUniformBuffer({materialData->modelBuffer},
                                           RED::ShaderType::VERTEX,
                                           0,
                                           1);

            drawCalls.push_back(dc);
            depthPassDrawCalls.push_back(depthDrawCall);
        });

    // Finish updating data to resources.
    // Start render pass.

    DepthPassInputParameters depthPassParameters;
    depthPassParameters.viewport         = viewportData;
    depthPassParameters.depthFrameBuffer = m_offscreenState.depthTexture.GetID();
    depthPassParameters.drawCalls        = std::move(depthPassDrawCalls);

    m_depthPassStage.Execute(m_device.get(), std::move(depthPassParameters), {});

    DrawOpaqueInputParameters drawOpaqueParameters;
    drawOpaqueParameters.colorFrameBuffer = m_offscreenState.colorTextures.at(0).GetID();
    drawOpaqueParameters.depthFrameBuffer = m_offscreenState.depthTexture.GetID();
    drawOpaqueParameters.viewport         = viewportData;
    drawOpaqueParameters.drawCalls        = std::move(drawCalls);

    m_lightForwardStage.Execute(m_device.get(), std::move(drawOpaqueParameters), {});

    if (m_rendererRegistry.ctx().contains<SkyData>())
    {
        auto&       sky     = m_rendererRegistry.ctx().get<SkyData>();
        const auto& skyMesh = m_rendererRegistry.try_get<MeshData>(sky.meshHandle);

        if (skyMesh)
        {
            SkyboxUniformData skyboxUniformData;
            skyboxUniformData.model = glm::translate(glm::mat4(1.0f), cameraData.cameraPosition);
            skyboxUniformData.color = glm::vec4(1.0f);
            skyboxUniformData.view  = cameraData.view;
            skyboxUniformData.projection = cameraData.projection;
            m_device->UpdateBuffer(sky.skyMeshDataBuffer.GetID(), &skyboxUniformData);

            RED::DrawCall dc;
            dc.vertexBuffer = skyMesh->m_vertexBuffer;
            dc.indexBuffer  = skyMesh->m_indexBuffer;
            dc.SetUniformBuffer({sky.skyMeshDataBuffer}, RED::ShaderType::VERTEX, 0);
            dc.SetUniformTexture({sky.skyTexture}, RED::ShaderType::FRAGMENT, 1, 1);

            SkyboxInputParameters skyboxParameters;
            skyboxParameters.viewport         = viewportData;
            skyboxParameters.colorFrameBuffer = m_offscreenState.colorTextures.at(0).GetID();
            skyboxParameters.depthFrameBuffer = m_offscreenState.depthTexture.GetID();
            skyboxParameters.drawCall         = std::move(dc);

            m_skyboxStage.Execute(m_device.get(), std::move(skyboxParameters), {});
        }
    }

#ifdef _DEBUG
    m_imgui->BeginImGUIRender();
    m_imgui->Render(registry, cameraData);

    RED::FrameBuffer attachment{.textureView    = m_offscreenState.colorTextures.at(0).GetID(),
                                .loadOperation  = RED::FrameBufferLoadOperation::LOAD,
                                .storeOperation = RED::FrameBufferStoreOperation::STORE,
                                .clearColor     = {.0f, .0f, .0f, .0f}};

    m_device->EnableRendering({viewportData.x,
                               viewportData.y,
                               static_cast<u32>(viewportData.width),
                               static_cast<u32>(viewportData.height)},
                              {attachment});

    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                    (VkCommandBuffer)m_device->GetCurrentCommandBuffer());

    m_device->DisableRendering();
#endif

    // End render pass.

    PresentInputParameters presentParameters{};
    presentParameters.quadBuffer       = m_offscreenState.quadBuffer;
    presentParameters.offscreenTexture = m_offscreenState.colorTextures.at(0).GetID();
    presentParameters.viewport         = viewportData;

    RED::GPUTextureView backbuffer;
    m_presentStage.Execute(m_device.get(), presentParameters, backbuffer);

    m_device->EndFrame();
}
} // namespace Pinut
