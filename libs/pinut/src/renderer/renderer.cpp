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
#include "src/renderer/meshData.h"
#include "src/renderer/renderer.h"
#include "src/renderer/swapchain.h"
#include "src/renderer/utils.h"

// TODO TEMP
#include "src/renderer/common.h"

namespace Pinut
{
RED::GPUBuffer uniformBuffer;
RED::GPUBuffer uniformColorBuffer;
RED::GPUBuffer quadBuffer;
PerFrameData   uniformData{};

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

    uniformBuffer = m_device->CreateBuffer({140, 140, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT});

    glm::vec4 color = glm::vec4(1, 0, 0, 0);
    uniformColorBuffer =
      m_device->CreateBuffer({64, 64, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT}, &color);

    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec2 uv;
    };

    // clang-format off
    std::array<QuadVertex, 6> quadData = {QuadVertex{glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
                                          QuadVertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
                                          QuadVertex{glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
                                          QuadVertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
                                          QuadVertex{glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
                                          QuadVertex{glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec2(1.0f, 0.0f)}};
    // clang-format on
    quadBuffer =
      m_device->CreateBuffer({120, 20, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT}, quadData.data());
}

Renderer::~Renderer()
{
    m_device->WaitIdle();
    quadBuffer.Destroy();
    uniformBuffer.Destroy();
    uniformColorBuffer.Destroy();
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
      });
}

void Renderer::Render(entt::registry& registry, const ViewportData& viewportData)
{
    m_device->BeginFrame();

    std::vector<VkRenderingAttachmentInfo> colorAttachments;
    colorAttachments.reserve(m_offscreenState.colorTextures.size());

    m_device->TransitionImageLayout(m_offscreenState.colorTextures.at(0).GetID(),
                                    0,
                                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    for (const auto& t : m_offscreenState.colorTextures)
    {
        if (t.IsEmpty())
            break;

        VkRenderingAttachmentInfo attachmentInfo =
          m_device->GetAttachment(t,
                                  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                  VK_ATTACHMENT_LOAD_OP_CLEAR,
                                  VK_ATTACHMENT_STORE_OP_STORE,
                                  {.0f, .0f, .0f, .0f});
        colorAttachments.emplace_back(attachmentInfo);
    }

    auto depthAttachment = m_device->GetAttachment(m_offscreenState.depthTexture,
                                                   VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                                   VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                   VK_ATTACHMENT_STORE_OP_STORE,
                                                   {1.0f, 0.0f});

    m_device->EnableRendering({viewportData.x,
                               viewportData.y,
                               static_cast<u32>(viewportData.width),
                               static_cast<u32>(viewportData.height)},
                              std::move(colorAttachments),
                              &depthAttachment);

    RED::ViewportState viewport{};
    viewport.x      = viewportData.x;
    viewport.y      = viewportData.y;
    viewport.width  = viewportData.width;
    viewport.height = viewportData.height;

    RED::GraphicsState graphicsState{};
    graphicsState.viewport = std::move(viewport);
    graphicsState.depth    = {VK_FORMAT_D32_SFLOAT};

    m_device->SetGraphicsState(&graphicsState);
    m_device->SetRenderPipeline(&m_pipelines.at("flat"));

    uniformData.view           = viewportData.view;
    uniformData.projection     = viewportData.projection;
    uniformData.cameraPosition = viewportData.cameraPosition;
    m_device->UpdateBuffer(uniformBuffer.GetID(), &uniformData);

    std::vector<RED::DrawCall> drawCalls;
    drawCalls.reserve(1000);

    registry.view<Component::TransformComponent, Component::RenderComponent>().each(
      [&drawCalls, this](auto entity, auto& transformComponent, auto& renderComponent)
      {
          const auto meshData = m_rendererRegistry.try_get<MeshData>(renderComponent.id);
          if (!meshData)
              return;

          m_device->UpdateBuffer(uniformColorBuffer.GetID(), &transformComponent.model);

          RED::DrawCall dc;
          dc.vertexBuffer = meshData->m_vertexBuffer;
          dc.indexBuffer  = meshData->m_indexBuffer;

          dc.SetUniformBuffer(uniformBuffer, RED::ShaderType::VERTEX, 0, 0);
          dc.SetUniformBuffer(uniformColorBuffer, RED::ShaderType::VERTEX, 0, 1);

          drawCalls.push_back(dc);
      });

    m_device->SubmitDrawCalls({drawCalls});

    m_device->DisableRendering();

    m_device->EndCommandRecording(); // This is probably unnecessary

    m_device->TransitionImageLayout(m_offscreenState.colorTextures.at(0).GetID(),
                                    0,
                                    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    m_device->BeginCommandRecording(RED::QueueType::GRAPHICS);

    VkRenderingAttachmentInfo attachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    attachment.imageView   = m_swapchain->imageViews.at(m_swapchain->imageIndex);
    attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.loadOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.clearValue  = {0.0f, 0.f, 0.f, 0.f};

    m_device->EnableRendering({viewportData.x,
                               viewportData.y,
                               static_cast<u32>(viewportData.width),
                               static_cast<u32>(viewportData.height)},
                              {attachment});

    graphicsState.depth = {VK_FORMAT_UNDEFINED};

    m_device->SetGraphicsState(&graphicsState);
    m_device->SetRenderPipeline(&m_pipelines.at("present"));

    RED::DrawCall dc;
    dc.vertexBuffer = quadBuffer;
    dc.SetUniformTexture(m_offscreenState.colorTextures.at(0), RED::ShaderType::FRAGMENT, 0);

    m_device->SubmitDrawCalls({dc});

    m_device->DisableRendering();

    m_device->EndFrame();
}
} // namespace Pinut
