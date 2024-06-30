#include "stdafx.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "application.h"
#include "src/renderer/common.h"
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

    // TODO Make number backbuffers a variable.
    m_device.OnCreate("Sandbox", "PinutEngine", ENABLE_GPU_VALIDATION_DEFAULT, m_window);
    m_swapchain.OnCreate(&m_device, 3, m_window);
    m_commandBufferManager.OnCreate(&m_device, 3);

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
    };
    m_descriptorSetManager.OnCreate(m_device.GetDevice(), 3, 2, std::move(descriptorPoolSizes));

    UpdateDisplay();
    Primitives::InitializeDefaultPrimitives(&m_device);

    m_forwardPipeline.Init(&m_device);

    m_perFrameBuffer.Create(&m_device,
                            sizeof(PerFrameData),
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VMA_MEMORY_USAGE_AUTO);

    m_perObjectBuffer.Create(&m_device,
                             sizeof(glm::mat4),
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VMA_MEMORY_USAGE_AUTO);
}

void Application::Shutdown()
{
    assert(vkDeviceWaitIdle(m_device.GetDevice()) == VK_SUCCESS);

    // TODO temporal
    m_perFrameBuffer.Destroy();
    m_perObjectBuffer.Destroy();
    // TODO end temporal

    Primitives::DestroyDefaultPrimitives();

    m_forwardPipeline.Shutdown();

    m_commandBufferManager.OnDestroy();
    m_descriptorSetManager.OnDestroy();

    m_swapchain.OnDestroy();
    m_device.OnDestroy();
    glfwDestroyWindow(m_window);
}

void Application::Render()
{
    auto frameIndex = m_swapchain.WaitForSwapchain();
    m_descriptorSetManager.Clear();

    // Draw
    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE}, renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence     fence{VK_NULL_HANDLE};
    m_swapchain.GetSyncObjects(&imageAvailableSemaphore, &renderFinishedSemaphore, &fence);

    m_commandBufferManager.OnBeginFrame();
    auto cmd = m_commandBufferManager.GetNewCommandBuffer();

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

    const auto perFrameDescriptorSet =
      m_descriptorSetManager.Allocate(m_forwardPipeline.PerFrameDescriptorSetLayout());
    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_forwardPipeline.PipelineLayout(),
                            0,
                            1,
                            &perFrameDescriptorSet,
                            0,
                            nullptr);

    auto perFrameData = (PerFrameData*)m_perFrameBuffer.AllocationInfo().pMappedData;
    perFrameData->view =
      glm::lookAt(glm::vec3(0.0f, 5.0f, -2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    perFrameData->projection =
      glm::perspective(glm::radians(60.0f), (float)m_width / m_height, 0.01f, 10000.0f);

    auto perFrameBufferInfo =
      vkinit::DescriptorBufferInfo(m_perFrameBuffer.m_buffer, 0, VK_WHOLE_SIZE);
    auto perFrameWrite = vkinit::WriteDescriptorSet(perFrameDescriptorSet,
                                                    0,
                                                    1,
                                                    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                    &perFrameBufferInfo);
    vkUpdateDescriptorSets(m_device.GetDevice(), 1, &perFrameWrite, 0, nullptr);

    const auto perObjectDescriptorSet =
      m_descriptorSetManager.Allocate(m_forwardPipeline.PerObjectDescriptorSetLayout());
    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_forwardPipeline.PipelineLayout(),
                            1,
                            1,
                            &perObjectDescriptorSet,
                            0,
                            nullptr);

    auto perObjectData = (glm::mat4*)m_perObjectBuffer.AllocationInfo().pMappedData;
    *perObjectData     = glm::mat4(1.0f);

    auto perObjectBufferInfo =
      vkinit::DescriptorBufferInfo(m_perObjectBuffer.m_buffer, 0, VK_WHOLE_SIZE);
    auto perObjectWrite = vkinit::WriteDescriptorSet(perObjectDescriptorSet,
                                                     0,
                                                     1,
                                                     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                     &perObjectBufferInfo);
    vkUpdateDescriptorSets(m_device.GetDevice(), 1, &perObjectWrite, 0, nullptr);

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

    auto ok = vkQueueSubmit(m_device.GetGraphicsQueue(), 1, &submitInfo, fence);
    assert(ok == VK_SUCCESS);

    m_swapchain.Present();
}

void Application::UpdateDisplay()
{
    m_swapchain.OnDestroyWindowDependantResources();
    m_swapchain.OnCreateWindowDependantResources(m_width, m_height);
}
} // namespace Pinut
