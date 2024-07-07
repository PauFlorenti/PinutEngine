#include "stdafx.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include <imgui.h>

#include "application.h"
#include "src/core/camera.h"
#include "src/core/scene.h"
#include "src/renderer/common.h"
#include "src/renderer/mesh.h"
#include "src/renderer/primitives.h"
#include "src/renderer/renderable.h"
#include "src/renderer/texture.h"
#include "src/renderer/utils.h"

#if _DEBUG
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = true;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = true;
#else // RELEASE
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = false;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = false;
#endif

static bool bMinimized{false};

Pinut::Renderable* renderable{nullptr};

i32 Run(Pinut::Application* application)
{
    assert(application);
    if (!application)
        return -1;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    auto window = glfwCreateWindow(application->GetWidth(),
                                   application->GetHeight(),
                                   application->GetName().c_str(),
                                   nullptr,
                                   nullptr);

    if (!window)
        return -1;

    glfwSetWindowUserPointer(window, application);
    glfwSetWindowSizeCallback(window, &Pinut::Application::OnWindowResized);
    glfwSetWindowPosCallback(window, &Pinut::Application::OnWindowMoved);

    application->Init(window);
    application->OnCreate();

    while (!glfwWindowShouldClose(window))
    {
        // Check if we should close
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        glfwPollEvents();

        if (!bMinimized)
        {
            application->Update();
            application->OnUpdate();
            application->OnRender();
            application->Render();
        }
    }

    application->OnDestroy();
    application->Shutdown();

    return 0;
}

namespace Pinut
{
Application::Application(const std::string& name, i32 width, i32 height)
: m_name(name),
  m_width(width),
  m_height(height)
{
}

void Application::OnWindowMoved(GLFWwindow* window, int x, int y) { printf("Window moved\n"); }

void Application::OnWindowResized(GLFWwindow* window, int width, int height)
{
    printf("Window resized [%d, %d]\n", width, height);

    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    assert(app != nullptr);

    if (app->m_width == width && app->m_height == height)
        return;

    if (width == 0 || height == 0)
    {
        bMinimized = true;
        return;
    }

    bMinimized = false;

    app->m_width  = width;
    app->m_height = height;

    app->UpdateDisplay();
}

void Application::Init(GLFWwindow* window)
{
    assert(window);
    if (!window)
        return;

    m_window = window;

    // Get framebuffer size
    i32 w, h;
    glfwGetWindowSize(m_window, &w, &h);
    m_width  = w;
    m_height = h;

    // TODO Make number backbuffers a variable.
    m_device.OnCreate("Sandbox", "PinutEngine", ENABLE_GPU_VALIDATION_DEFAULT, m_window);
    m_swapchain.OnCreate(&m_device, 3, m_window);
    m_commandBufferManager.OnCreate(&m_device, 3);

    Primitives::InitializeDefaultPrimitives(&m_device);
    m_forwardPipeline.Init(&m_device);

    UpdateDisplay();

    renderable = new Renderable();
    renderable->SetMesh(Primitives::GetUnitCube());

#ifdef _DEBUG
    m_imgui.Init(&m_device, &m_swapchain, window);
#endif
}

void Application::Shutdown()
{
    assert(vkDeviceWaitIdle(m_device.GetDevice()) == VK_SUCCESS);

#ifdef _DEBUG
    m_imgui.Shutdown();
#endif

    Primitives::DestroyDefaultPrimitives();
    m_forwardPipeline.Shutdown();
    m_commandBufferManager.OnDestroy();
    m_swapchain.OnDestroy();
    m_device.OnDestroy();
    glfwDestroyWindow(m_window);
}

void Application::Update()
{
    static auto startTime = glfwGetTime();

    auto currentTime = glfwGetTime();
    m_deltaTime      = currentTime - m_lastFrameTime;
    m_lastFrameTime  = currentTime;
}

void Application::Render()
{
    auto frameIndex = m_swapchain.WaitForSwapchain();

    // Draw
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE}, renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence     fence{VK_NULL_HANDLE};
    m_swapchain.GetSyncObjects(&imageAvailableSemaphore, &renderFinishedSemaphore, &fence);

    m_commandBufferManager.OnBeginFrame();
    const auto cmd = m_commandBufferManager.GetNewCommandBuffer();

    const auto cmdBeginInfo =
      vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    assert(vkBeginCommandBuffer(cmd, &cmdBeginInfo) == VK_SUCCESS);

    auto depthTexture = m_forwardPipeline.GetDepthAttachment();

    Texture::TransitionImageLayout(cmd,
                                   m_swapchain.GetCurrentImage(),
                                   0,
                                   VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                   VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    Texture::TransitionImageLayout(cmd,
                                   depthTexture->Image(),
                                   0,
                                   VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                   VK_IMAGE_LAYOUT_UNDEFINED,
                                   VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1});

    auto attachment = vkinit::RenderingAttachmentInfo(m_swapchain.GetCurrentImageView(),
                                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                      VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                      VK_ATTACHMENT_STORE_OP_STORE,
                                                      {0.0f, 0.f, 0.f, 0.f});

    auto depthAttachment = vkinit::RenderingAttachmentInfo(depthTexture->ImageView(),
                                                           VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                                                           VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                           VK_ATTACHMENT_STORE_OP_STORE,
                                                           {1.0, 0});

    const auto renderingInfo =
      vkinit::RenderingInfo(1, &attachment, {0, 0, m_width, m_height}, &depthAttachment);

    vkCmdBeginRendering(cmd, &renderingInfo);

    m_forwardPipeline.BindPipeline(cmd);

    VkRect2D scissors{};
    scissors.extent = {m_width, m_height};
    scissors.offset = {0, 0};

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = static_cast<float>(m_height);
    viewport.width    = static_cast<float>(m_width);
    viewport.height   = -static_cast<float>(m_height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissors);

    PerFrameData perFrameData{};

    perFrameData.view           = m_currentCamera->View();
    perFrameData.projection     = m_currentCamera->Projection();
    perFrameData.cameraPosition = m_currentCamera->Position();

    m_forwardPipeline.UpdatePerFrameData(cmd, std::move(perFrameData));

    m_forwardPipeline.Render(cmd, m_currentScene);

    vkCmdEndRendering(cmd);

#ifdef _DEBUG
    m_imgui.BeginImGUIRender(cmd);
    ImGui::ShowDemoWindow();
    m_imgui.EndImGUIRender(cmd, m_width, m_height, m_swapchain.GetCurrentImageView());
#endif

    Texture::TransitionImageLayout(cmd,
                                   m_swapchain.GetCurrentImage(),
                                   VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                   VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                   VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                   {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    vkEndCommandBuffer(cmd);

    VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submitInfo{
      .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount   = 1,
      .pWaitSemaphores      = &imageAvailableSemaphore,
      .pWaitDstStageMask    = &stage,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &cmd,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores    = &renderFinishedSemaphore,
    };

    auto ok = vkQueueSubmit(m_device.GetGraphicsQueue(), 1, &submitInfo, fence);
    assert(ok == VK_SUCCESS);

    m_swapchain.Present();
}

void Application::UpdateDisplay()
{
    m_swapchain.OnDestroyWindowDependantResources();
    m_forwardPipeline.OnDestroyWindowDependantResources();
    m_swapchain.OnCreateWindowDependantResources(m_width, m_height);
    m_forwardPipeline.OnCreateWindowDependantResources(m_width, m_height);
}

Camera* Application::GetCamera()
{
    if (!m_currentCamera)
        m_currentCamera = new Camera();

    return m_currentCamera;
}
} // namespace Pinut
