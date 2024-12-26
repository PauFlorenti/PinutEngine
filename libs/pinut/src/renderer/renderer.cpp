#include "stdafx.h"

#include <external/vk-bootstrap/src/VkBootstrap.h>
#define GLFW_INCLUDE_VULKAN
#include <external/glfw/include/GLFW/glfw3.h>
#include <external/tinygltf/json.hpp>

#include "render_device/bufferDescriptor.h"
#include "render_device/drawCall.h"
#include "render_device/shader.h"
#include "render_device/states.h"
#include "render_device/textureDescriptor.h"

#include "src/assets/mesh.h"
#include "src/components/meshComponent.h"
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

Renderer::Renderer(std::shared_ptr<RED::Device> device, SwapchainInfo* swapchain)
: m_device(device),
  m_swapchain(swapchain)
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

    registry.view<Component::RenderComponent, Component::MeshComponent>().each(
      [this](entt::entity entity, auto& renderComponent, auto& meshComponent)
      {
          if (m_rendererRegistry.try_get<MeshData>(renderComponent.id))
              return;

          renderComponent.id = m_rendererRegistry.create();
          const auto& mesh   = meshComponent.mesh;

          auto& data = m_rendererRegistry.emplace<MeshData>(renderComponent.id);

          auto vertices       = mesh.m_vertices;
          data.m_vertexBuffer = m_device->CreateBuffer(
            {vertices.size() * sizeof(Vertex), sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
            vertices.data());

          if (!mesh.m_indices.empty())
          {
              auto indices       = mesh.m_indices;
              data.m_indexBuffer = m_device->CreateBuffer(
                {indices.size() * sizeof(u16), sizeof(u16), VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
                indices.data());
          }

          if (!m_rendererRegistry.try_get<MaterialData>(renderComponent.id))
          {
              auto& materialData = m_rendererRegistry.emplace<MaterialData>(renderComponent.id);

              RED::TextureDescriptor textureDescriptor{
                {renderComponent.difuse.GetWidth(), renderComponent.difuse.GetHeight(), 1},
                renderComponent.difuse.GetFormat(),
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_USAGE_SAMPLED_BIT};
              materialData.difuseTexture =
                m_device->CreateTexture(textureDescriptor, renderComponent.difuse.GetData());

              materialData.uniformBuffer =
                m_device->CreateBuffer({64, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT});
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

    m_device->UpdateBuffer(m_offscreenState.globalUniformBuffer.GetID(), &viewportData.cameraData);

    RED::ViewportState viewport{viewportData.x,
                                viewportData.y,
                                viewportData.width,
                                viewportData.height};

    std::vector<RED::DrawCall> drawCalls;
    drawCalls.reserve(1000);

    registry.view<Component::TransformComponent, Component::RenderComponent>().each(
      [&drawCalls, this](auto entity, auto& transformComponent, auto& renderComponent)
      {
          const auto meshData     = m_rendererRegistry.try_get<MeshData>(renderComponent.id);
          const auto materialData = m_rendererRegistry.try_get<MaterialData>(renderComponent.id);

          if (!meshData || !materialData)
              return;

          m_device->UpdateBuffer(materialData->uniformBuffer.GetID(), &transformComponent.model);

          RED::DrawCall dc;
          dc.vertexBuffer = meshData->m_vertexBuffer;
          dc.indexBuffer  = meshData->m_indexBuffer;

          dc.SetUniformBuffer(m_offscreenState.globalUniformBuffer, RED::ShaderType::VERTEX, 0, 0);
          dc.SetUniformBuffer(materialData->uniformBuffer, RED::ShaderType::VERTEX, 0, 1);
          dc.SetUniformTexture(materialData->difuseTexture, RED::ShaderType::FRAGMENT, 1, 1);

          drawCalls.push_back(dc);
      });

    // Finish updating data to resources.
    // Start render pass.

    DrawOpaqueInputParameters drawOpaqueParameters;
    drawOpaqueParameters.colorFrameBuffer = m_offscreenState.colorTextures.at(0).GetID();
    drawOpaqueParameters.depthFrameBuffer = m_offscreenState.depthTexture.GetID();
    drawOpaqueParameters.viewport         = viewportData;
    drawOpaqueParameters.drawCalls        = std::move(drawCalls);

    m_lightForwardStage.Execute(m_device.get(), std::move(drawOpaqueParameters), {});

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
