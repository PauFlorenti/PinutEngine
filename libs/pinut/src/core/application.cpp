#include "stdafx.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include "application.h"
#include "src/renderer/mesh.h"
#include "src/renderer/primitives.h"
#include "src/renderer/utils.h"

#if _DEBUG
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = true;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = true;
#else // RELEASE
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = false;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = false;
#endif

static bool bMinimized{false};

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

    m_device.OnCreate("Sandbox", "PinutEngine", ENABLE_GPU_VALIDATION_DEFAULT, m_window);
    m_swapchain.OnCreate(&m_device, 3, m_window);

    // TODO Temporal. We should create a commandBufferManager.
    VkCommandPoolCreateInfo commandPoolInfo{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = m_device.GetGraphicsQueueIndex(),
    };

    auto ok = vkCreateCommandPool(m_device.GetDevice(), &commandPoolInfo, nullptr, &m_commandPool);
    assert(ok == VK_SUCCESS);

    VkCommandBufferAllocateInfo allocateInfo{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = m_commandPool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 3,
    };

    ok = vkAllocateCommandBuffers(m_device.GetDevice(), &allocateInfo, cmds);
    assert(ok == VK_SUCCESS);
    // TODO Finish Temporal.

    UpdateDisplay();
    Primitives::InitializeDefaultPrimitives(&m_device);

    m_forwardPipeline.Init(&m_device);
}

void Application::Shutdown()
{
    assert(vkDeviceWaitIdle(m_device.GetDevice()) == VK_SUCCESS);

    Primitives::DestroyDefaultPrimitives();

    m_forwardPipeline.Shutdown();

    vkDestroyCommandPool(m_device.GetDevice(), m_commandPool, nullptr);

    m_swapchain.OnDestroy();
    m_device.OnDestroy();
    glfwDestroyWindow(m_window);
}

void Application::Render()
{
    auto frameIndex = m_swapchain.WaitForSwapchain();

    // Draw
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE}, renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence     fence{VK_NULL_HANDLE};
    m_swapchain.GetSyncObjects(&imageAvailableSemaphore, &renderFinishedSemaphore, &fence);

    auto ok = vkResetCommandPool(m_device.GetDevice(), m_commandPool, 0);
    assert(ok == VK_SUCCESS);

    auto& cmd = cmds[m_swapchain.GetImageIndex()];

    auto cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    assert(vkBeginCommandBuffer(cmd, &cmdBeginInfo) == VK_SUCCESS);

    {
        VkImageMemoryBarrier2 barrier{
          .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
          .srcStageMask     = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
          .srcAccessMask    = 0,
          .dstStageMask     = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
          .dstAccessMask    = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
          .oldLayout        = VK_IMAGE_LAYOUT_UNDEFINED,
          .newLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
          .image            = m_swapchain.GetCurrentImage(),
          .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };

        VkDependencyInfo dependencyInfo{
          .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
          .imageMemoryBarrierCount = 1,
          .pImageMemoryBarriers    = &barrier,
        };

        vkCmdPipelineBarrier2(cmd, &dependencyInfo);
    }

    auto attachment = vkinit::RenderingAttachmentInfo(m_swapchain.GetCurrentImageView(),
                                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                                      VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                      VK_ATTACHMENT_STORE_OP_STORE,
                                                      {0.f, 0.f, 0.f, 0.f});

    auto renderingInfo = vkinit::RenderingInfo(1, &attachment, {{0, 0}, {m_width, m_height}});

    VkRect2D scissors{};
    scissors.extent = {m_width, m_height};
    scissors.offset = {0, 0};

    VkRenderingInfo renderingInfoTest{
      .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
      .renderArea           = {0, 0, m_width, m_height},
      .layerCount           = 1,
      .colorAttachmentCount = 1,
      .pColorAttachments    = &attachment,
      .pDepthAttachment     = nullptr,
      .pStencilAttachment   = nullptr,
    };

    vkCmdBeginRendering(cmd, &renderingInfo);

    VkViewport viewport{};
    viewport.width    = static_cast<float>(m_width);
    viewport.height   = static_cast<float>(m_height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);
    vkCmdSetScissor(cmd, 0, 1, &scissors);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_forwardPipeline.Pipeline());

    VkDeviceSize offset{0};
    auto         cube = Primitives::GetUnitCube();
    vkCmdBindVertexBuffers(cmd, 0, 1, &cube->m_vertexBuffer.m_buffer, &offset);
    vkCmdBindIndexBuffer(cmd, cube->m_indexBuffer.m_buffer, offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, cube->GetIndexCount(), 1, 0, 0, 0);

    vkCmdEndRendering(cmd);

    {
        VkImageMemoryBarrier2 barrier{
          .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
          .srcStageMask     = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
          .srcAccessMask    = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
          .dstStageMask     = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
          .dstAccessMask    = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
          .oldLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
          .newLayout        = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
          .image            = m_swapchain.GetCurrentImage(),
          .subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        };

        VkDependencyInfo dependencyInfo{
          .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
          .imageMemoryBarrierCount = 1,
          .pImageMemoryBarriers    = &barrier,
        };

        vkCmdPipelineBarrier2(cmd, &dependencyInfo);
    }

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

    ok = vkQueueSubmit(m_device.GetGraphicsQueue(), 1, &submitInfo, fence);
    assert(ok == VK_SUCCESS);

    m_swapchain.Present();
}

void Application::UpdateDisplay()
{
    m_swapchain.OnDestroyWindowDependantResources();
    m_swapchain.OnCreateWindowDependantResources(m_width, m_height);
}
} // namespace Pinut
