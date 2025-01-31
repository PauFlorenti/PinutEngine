#include "stdafx.h"

#include <external/vk-bootstrap/src/VkBootstrap.h>
#define GLFW_INCLUDE_VULKAN
#include <external/glfw/include/GLFW/glfw3.h>
#include <external/imgui/backends/imgui_impl_vulkan.h>
#include <external/tinygltf/json.hpp>

#include "render_device/bufferDescriptor.h"
#include "render_device/device.h"
#include "render_device/drawCall.h"
#include "render_device/shader.h"
#include "render_device/states.h"
#include "render_device/textureDescriptor.h"

#include "src/assets/mesh.h"
#include "src/components/lightComponent.h"
#include "src/components/meshComponent.h"
#include "src/imgui/pinutImgui.h"
#include "src/renderer/materialData.h"
#include "src/renderer/meshData.h"
#include "src/renderer/renderer.h"
#include "src/renderer/swapchain.h"
#include "src/renderer/utils.h"

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

std::vector<VkFormat> PopulateAttachmentsFromJson(const nlohmann::json& j)
{
    if (!j.contains("attachments"))
        return {};

    const auto            jAttachments = j["attachments"];
    std::vector<VkFormat> formats;
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
  m_swapchain(swapchain)
#ifdef _DEBUG
  ,
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
                                {VK_FORMAT_R32G32B32A32_SFLOAT},
                                true,
                                VK_FORMAT_D32_SFLOAT);
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

          const auto m                       = transformComponent.model;
          lightData.at(lightIndex).color     = lightComponent.m_color;
          lightData.at(lightIndex).intensity = lightComponent.m_intensity;
          lightData.at(lightIndex).position  = glm::vec3(m[3]);
          lightData.at(lightIndex).radius    = lightComponent.m_radius;
          lightData.at(lightIndex).direction = glm::normalize(glm::vec3(m[2][0], m[2][1], m[2][2]));
          lightData.at(lightIndex).innerCone = lightComponent.m_innerCone;
          lightData.at(lightIndex).outerCone = lightComponent.m_outerCone;
          lightData.at(lightIndex).cosineExponent = lightComponent.m_cosineExponent;

          ++lightIndex;
      });
    m_device->UpdateBuffer(m_offscreenState.lightsBuffer.GetID(), lightData.data());

    registry.view<Component::RenderComponent, Component::MeshComponent>().each(
      [this](entt::entity entity, auto& renderComponent, auto& meshComponent)
      {
          if (m_rendererRegistry.try_get<MeshData>(renderComponent.id))
              return;

          renderComponent.id = m_rendererRegistry.create();
          auto& mesh         = meshComponent.mesh;

          auto& data = m_rendererRegistry.emplace<MeshData>(renderComponent.id);

          auto& vertices      = mesh.m_vertices;
          data.m_vertexBuffer = m_device->CreateBuffer(
            {vertices.size() * sizeof(Vertex), sizeof(Vertex), RED::BufferUsage::VERTEX},
            vertices.data());

          if (!mesh.m_indices.empty())
          {
              auto& indices      = mesh.m_indices;
              data.m_indexBuffer = m_device->CreateBuffer(
                {indices.size() * sizeof(u16), sizeof(u16), RED::BufferUsage::INDEX},
                indices.data());
          }

          if (!m_rendererRegistry.try_get<MaterialData>(renderComponent.id))
          {
              auto& materialData = m_rendererRegistry.emplace<MaterialData>(renderComponent.id);

              auto CreateMaterialDataTexture = [&device = this->m_device](Texture t)
              {
                  RED::TextureDescriptor textureDescriptor{{t.GetWidth(), t.GetHeight(), 1},
                                                           t.GetFormat(),
                                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                                           VK_IMAGE_USAGE_SAMPLED_BIT};
                  return device->CreateTexture(textureDescriptor, t.GetData());
              };

              materialData.difuseTexture = CreateMaterialDataTexture(renderComponent.difuse);
              materialData.normalTexture = CreateMaterialDataTexture(renderComponent.normal);
              materialData.metallicRoughnessTexture =
                CreateMaterialDataTexture(renderComponent.metallicRoughness);
              materialData.emissiveTexture = CreateMaterialDataTexture(renderComponent.emissive);

              materialData.modelBuffer =
                m_device->CreateBuffer({64, 64, RED::BufferUsage::UNIFORM});

              u32 uniformMaterialData[4] = {0xFFFFFFFF, 0x00000000, 0, 0};
              materialData.uniformBuffer =
                m_device->CreateBuffer({16, 16, RED::BufferUsage::UNIFORM}, &uniformMaterialData);
          }
      });
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

    RED::ViewportState viewport{viewportData.x,
                                viewportData.y,
                                viewportData.width,
                                viewportData.height};

    std::vector<RED::DrawCall> drawCalls;
    drawCalls.reserve(1000);
    std::vector<RED::DrawCall> depthPassDrawCalls;
    depthPassDrawCalls.reserve(1000);

    registry.view<Component::TransformComponent, Component::RenderComponent>().each(
      [&drawCalls, &depthPassDrawCalls, this](auto  entity,
                                              auto& transformComponent,
                                              auto& renderComponent)
      {
          const auto meshData     = m_rendererRegistry.try_get<MeshData>(renderComponent.id);
          const auto materialData = m_rendererRegistry.try_get<MaterialData>(renderComponent.id);

          if (!meshData || !materialData)
              return;

          m_device->UpdateBuffer(materialData->modelBuffer.GetID(), &transformComponent.model);

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

#ifdef _DEBUG
    m_imgui->BeginImGUIRender();

    RED::FrameBuffer attachment{.textureView    = m_offscreenState.colorTextures.at(0).GetID(),
                                .loadOperation  = RED::FrameBufferLoadOperation::LOAD,
                                .storeOperation = RED::FrameBufferStoreOperation::STORE,
                                .clearColor     = {.0f, .0f, .0f, .0f}};

    m_device->EnableRendering({viewportData.x,
                               viewportData.y,
                               static_cast<u32>(viewportData.width),
                               static_cast<u32>(viewportData.height)},
                              {attachment});

    ImGui::ShowDebugLogWindow();

    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                    (VkCommandBuffer)m_device->GetCurrentCommandBuffer());

    m_device->DisableRendering();
#endif

    // End render pass.

    PresentInputParameters presentParameters{};
    presentParameters.quadBuffer       = m_offscreenState.quadBuffer;
    presentParameters.offscreenTexture = m_offscreenState.colorTextures.at(0).GetID();
    presentParameters.viewport         = std::move(viewport);

    RED::GPUTextureView backbuffer;
    m_presentStage.Execute(m_device.get(), presentParameters, backbuffer);

    m_device->EndFrame();
}
} // namespace Pinut
