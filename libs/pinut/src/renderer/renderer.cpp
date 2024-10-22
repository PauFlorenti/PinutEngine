#include "stdafx.h"

#include <external/vk-bootstrap/src/VkBootstrap.h>
#define GLFW_INCLUDE_VULKAN
#include <external/glfw/include/GLFW/glfw3.h>
#include <external/tinygltf/json.hpp>

#include "render_device/bufferDescriptor.h"
#include "render_device/drawCall.h"
#include "render_device/shader.h"
#include "render_device/states.h"
#include "src/renderer/renderer.h"
#include "src/renderer/swapchain.h"

// TODO TEMP
#include "src/renderer/common.h"

namespace Pinut
{
RED::DrawCall  dc;
RED::GPUBuffer uniformBuffer;

RED::Shader PopulateShaderFromJson(const nlohmann::json& j, RED::ShaderType type)
{
    RED::Shader shader{j["name"].get<std::string>(), type};

    if (j.contains("uniforms"))
    {
        const auto& jUniforms = j["uniforms"];
        shader.uniformDataSlots.reserve(jUniforms.size());
        for (const auto& uniform : j["uniforms"])
        {
            const auto set = uniform["set"].get<u32>();
            shader.uniformDataSlots.emplace_back(type,
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

Renderer::Renderer(std::shared_ptr<RED::Device> device,
                   SwapchainInfo*               swapchain,
                   GLFWwindow*                  window,
                   i32                          width,
                   i32                          height)
: m_device(device),
  m_swapchain(swapchain),
  m_window(window),
  m_width(width),
  m_height(height)
{
    glfwSetWindowUserPointer(m_window, this);
    glfwSetWindowSizeCallback(m_window, &Renderer::OnWindowResized);

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
                             jdata.value("input_vertex", "Pos")}});
    }

    std::array<glm::vec3, 3> vertices = {glm::vec3{-0.5f, -0.5f, 0.0f},
                                         {0.5f, -0.5f, 0.0f},
                                         {0.0f, 0.5f, 0.0f}};

    RED::BufferDescriptor vertexBufferDescriptor{};
    vertexBufferDescriptor.elementSize = sizeof(glm::vec3);
    vertexBufferDescriptor.size        = sizeof(glm::vec3) * 3;
    vertexBufferDescriptor.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    dc.vertexCount  = 3;
    dc.vertexBuffer = m_device->CreateBuffer(vertexBufferDescriptor, vertices.data());

    RED::BufferDescriptor uniformBufferDescriptor{};
    uniformBufferDescriptor.elementSize = 128;
    uniformBufferDescriptor.size        = 128;
    uniformBufferDescriptor.usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    PerFrameData uniformData{};
    uniformData.view =
      glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    uniformData.projection =
      glm::perspective(60.0f, static_cast<f32>(m_width) / m_height, 0.001f, 1000.0f);
    uniformData.cameraPosition = glm::vec3(0.0f, 0.0f, -2.0f);

    uniformBuffer = m_device->CreateBuffer(std::move(uniformBufferDescriptor), &uniformData);

    dc.SetUniformBuffer(uniformBuffer, RED::ShaderType::VERTEX, 0, 0);
}

Renderer::~Renderer()
{
    m_device->WaitIdle();
    uniformBuffer.Destroy();
    dc.vertexBuffer.Destroy();

    m_device->OnDestroy();
}

void Renderer::Update()
{
    m_device->BeginFrame();

    m_device->TransitionImageLayout(m_swapchain->images.at(m_swapchain->imageIndex),
                                    0,
                                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    VkRenderingAttachmentInfo attachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    attachment.imageView   = m_swapchain->imageViews.at(m_swapchain->imageIndex);
    attachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.clearValue  = {1.0f, 0.f, 0.f, 0.f};

    m_device->EnableRendering({0, 0, static_cast<u32>(m_width), static_cast<u32>(m_height)},
                              {attachment});

    RED::ViewportState viewport{};
    viewport.x      = 0;
    viewport.y      = 0;
    viewport.width  = m_width;
    viewport.height = m_height;

    RED::GraphicsState graphicsState{};
    graphicsState.viewport = std::move(viewport);

    m_device->SetGraphicsState(&graphicsState);
    m_device->SetRenderPipeline(&m_pipelines.at("flat"));

    PerFrameData uniformData{};
    uniformData.view =
      glm::lookAt(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    uniformData.projection =
      glm::perspective(60.0f, static_cast<f32>(m_width) / m_height, 0.001f, 1000.0f);
    uniformData.cameraPosition = glm::vec3(0.0f, 0.0f, -2.0f);
    glm::translate(uniformData.view, glm::vec3(0.0f, 0.0f, -0.01f));
    m_device->UpdateBuffer(uniformBuffer, &uniformData);

    m_device->SubmitDrawCalls({dc});

    m_device->DisableRendering();

    m_device->TransitionImageLayout(m_swapchain->images.at(m_swapchain->imageIndex),
                                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    m_device->EndFrame();
    m_device->Present();
}

void Renderer::OnWindowResized(GLFWwindow* window, i32 width, i32 height)
{
    printf("Window resized [%d, %d]\n", width, height);

    auto renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    assert(renderer != nullptr);

    if (renderer->m_width == width && renderer->m_height == height)
        return;

    if (width == 0 || height == 0)
    {
        renderer->bMinimized = true;
        return;
    }

    renderer->bMinimized = false;

    renderer->m_width  = width;
    renderer->m_height = height;

    // renderer->m_swapchain->OnDestroyWindowDependantResources();
    // renderer->m_swapchain->OnCreateWindowDependantResources(width, height);
}
} // namespace Pinut
