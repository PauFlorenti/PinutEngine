#include "stdafx.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include "application.h"
#include "utils.h"

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
    printf("Window resized\n");

    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    assert(app != nullptr);

    if (app->m_width == width && app->m_height == height)
        return;

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
    glfwGetWindowSize(m_window, &m_width, &m_height);

    m_device.OnCreate("Sandbox", "PinutEngine", ENABLE_GPU_VALIDATION_DEFAULT, m_window);
    m_swapchain.OnCreate(&m_device, 3, m_window);

    // TODO Temporal. We should create a commandBufferManager.
    VkCommandPoolCreateInfo commandPoolInfo{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .queueFamilyIndex = m_device.GetGraphicsQueueIndex(),
    };

    auto ok = vkCreateCommandPool(m_device.GetDevice(), &commandPoolInfo, nullptr, &commandPool);
    assert(ok == VK_SUCCESS);

    UpdateDisplay();
}

void Application::Shutdown()
{
    m_swapchain.OnDestroy();
    m_device.OnDestroy();
    glfwDestroyWindow(m_window);
}

void Application::Render()
{
    auto frameIndex = m_swapchain.WaitForSwapchain();

    // Draw
    VkCommandBufferAllocateInfo allocateInfo{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = commandPool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    auto            ok = vkAllocateCommandBuffers(m_device.GetDevice(), &allocateInfo, &cmd);
    assert(ok == VK_SUCCESS);

    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE}, renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence     fence{VK_NULL_HANDLE};
    m_swapchain.GetSyncObjects(&imageAvailableSemaphore, &renderFinishedSemaphore, &fence);

    ok = vkResetCommandPool(m_device.GetDevice(), commandPool, 0);
    assert(ok == VK_SUCCESS);

    auto cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    assert(vkBeginCommandBuffer(cmd, &cmdBeginInfo) == VK_SUCCESS);

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
