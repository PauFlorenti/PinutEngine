#include "stdafx.h"

#include <external/vk-bootstrap/src/VkBootstrap.h>
#define GLFW_INCLUDE_VULKAN
#include <external/glfw/include/GLFW/glfw3.h>
#include <external/tinygltf/json.hpp>

#include "render_device/bufferDescriptor.h"
#include "render_device/drawCall.h"
#include "render_device/states.h"
#include "src/renderer/renderer.h"
#include "src/renderer/shaders/flat_fs.h"
#include "src/renderer/shaders/flat_vs.h"

namespace Pinut
{
vkb::Result<vkb::Instance> Renderer::CreateInstance()
{
    vkb::InstanceBuilder instanceBuilder;

    auto messaage_severity = //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    return instanceBuilder.set_app_name("")
      .set_debug_messenger_severity(messaage_severity)
#ifdef _DEBUG
      .request_validation_layers(true)
#else
      .request_validation_layers(false)
#endif
      .use_default_debug_messenger()
      .require_api_version(1, 3, 0)
      .build();
}

VkSurfaceKHR Renderer::CreateSurface(const VkInstance& instance, GLFWwindow* window)
{
    VkSurfaceKHR surface{VK_NULL_HANDLE};

    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        printf("[ERROR]: Failed to create surface.");
    }

    return surface;
}

vkb::Result<vkb::Device> Renderer::CreateDevice(const vkb::Instance& vkbInstance,
                                                const VkSurfaceKHR&  surface)
{
    VkPhysicalDeviceVulkan12Features features_12{
      .bufferDeviceAddress = true,
    };

    VkPhysicalDeviceVulkan13Features features_13{
      .synchronization2 = true,
      .dynamicRendering = true,
    };

    vkb::PhysicalDeviceSelector gpu_selector{vkbInstance};
    auto                        gpu_result = gpu_selector.set_minimum_version(1, 1)
                        .set_required_features_12(features_12)
                        .set_required_features_13(features_13)
                        .set_surface(surface)
                        .select();

    if (!gpu_result)
    {
        printf("[ERROR]: %s", gpu_result.error().message().c_str());
        return {gpu_result.error()};
    }

    auto gpu = gpu_result.value();

    VkPhysicalDeviceShaderDrawParametersFeatures shader_draw_parameters_features = {
      .sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETER_FEATURES,
      .shaderDrawParameters = VK_TRUE,
    };

    vkb::DeviceBuilder device_builder{gpu};
    return device_builder.add_pNext(&shader_draw_parameters_features).build();
}

Renderer::DeviceQueues Renderer::CreateDeviceQueues(const vkb::Device& vkbDevice)
{
    auto GetQueue = [](const vkb::Device& vkbDevice, vkb::QueueType vkbQueueType)
    {
        auto vkbQueueResult = vkbDevice.get_queue(vkbQueueType);
        if (!vkbQueueResult)
        {
            printf("[ERROR]: Failed to retrieve desired queue.");
            return QueueInfo{};
        }

        auto vkbQueueIndex = vkbDevice.get_queue_index(vkbQueueType);
        if (!vkbQueueIndex)
        {
            printf("[ERROR]: Failed to retrieve desired queue index.");
            return QueueInfo{};
        }

        return QueueInfo{vkbQueueResult.value(), vkbQueueIndex.value()};
    };

    return {
      GetQueue(vkbDevice, vkb::QueueType::graphics),
      GetQueue(vkbDevice, vkb::QueueType::present),
      GetQueue(vkbDevice, vkb::QueueType::compute),
    };
}

Renderer::SwapchainInfo Renderer::CreateSwapchain(const vkb::Device& vkbDevice,
                                                  const QueueInfo&   queueInfos,
                                                  bool               vsyncEnabled)
{
    vkb::SwapchainBuilder builder{vkbDevice};
    auto                  result =
      builder
        .set_desired_present_mode(vsyncEnabled ? VK_PRESENT_MODE_FIFO_KHR :
                                                 VK_PRESENT_MODE_IMMEDIATE_KHR)
        .set_desired_format({VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
        .build();

    if (!result)
    {
        printf("[ERROR]: Failed to create swapchain.");
        return {};
    }

    auto vkbSwapchain = result.value();

    return {
      .swapchain     = vkbSwapchain.swapchain,
      .surfaceFormat = {vkbSwapchain.image_format, vkbSwapchain.color_space},
      .surfaceExtent = vkbSwapchain.extent,
      .images        = vkbSwapchain.get_images().value(),
      .imageViews    = vkbSwapchain.get_image_views().value(),
      .imageIndex    = 0,
      .vsyncEnabled  = vsyncEnabled,
    };
}

void Renderer::DestroySwapchain(VkDevice& device, const SwapchainInfo& swapchainInfo)
{
    for (auto& imageView : swapchainInfo.imageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    if (swapchainInfo.swapchain)
    {
        vkDestroySwapchainKHR(device, swapchainInfo.swapchain, nullptr);
    }
}

void Renderer::BeginFrameCallback(void* context, VkSemaphore imageAvailableSemaphore)
{
    auto renderer = reinterpret_cast<Renderer*>(context);
    auto ok       = vkAcquireNextImageKHR(renderer->m_deviceInfo.device,
                                    renderer->m_swapchainInfo.swapchain,
                                    UINT64_MAX,
                                    imageAvailableSemaphore,
                                    VK_NULL_HANDLE,
                                    &renderer->m_swapchainInfo.imageIndex);

    if (ok == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // TODO Recreate
        renderer->RecreateSwapchain(renderer->m_swapchainInfo.vsyncEnabled);

        vkAcquireNextImageKHR(renderer->m_deviceInfo.device,
                              renderer->m_swapchainInfo.swapchain,
                              UINT64_MAX,
                              imageAvailableSemaphore,
                              VK_NULL_HANDLE,
                              &renderer->m_swapchainInfo.imageIndex);
    }
}

void Renderer::EndFrameCallback(void* context, VkSemaphore renderFinishedSemaphore)
{
    auto renderer                 = reinterpret_cast<Renderer*>(context);
    renderer->m_endFrameSemaphore = renderFinishedSemaphore;
}

RED::DrawCall dc;

Renderer::Renderer(GLFWwindow* window, i32 width, i32 height)
: m_window(window),
  m_width(width),
  m_height(height)
{
    if (!SetupVulkan())
        return;

    m_device = RED::Device::Create(&m_deviceInfo, m_deviceQueues.data(), &m_callbacks);

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
    catch (nlohmann::json::parse_error& e)
    {
        pipelineFile.close();
        printf("[ERROR]: Could not parse pipelines.json file.");
        return;
    }

    for (auto& j : jPipelineFile.items())
    {
        auto jdata = j.value();

        const auto vs          = jdata["vs"].get<std::string>();
        const auto fs          = jdata["fs"].get<std::string>();
        const auto inputVertex = jdata.value("input_vertex", "Pos");

        m_pipelines.insert({j.key(),
                            {j.key().c_str(),
                             {vs, RED::ShaderType::VERTEX},
                             {fs, RED::ShaderType::FRAGMENT},
                             inputVertex}});
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
}

void Renderer::Update()
{
    m_device->BeginFrame();

    m_device->TransitionImageLayout(m_swapchainInfo.images.at(m_swapchainInfo.imageIndex),
                                    0,
                                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    VkRenderingAttachmentInfo attachment{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO};
    attachment.imageView   = m_swapchainInfo.imageViews.at(m_swapchainInfo.imageIndex);
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

    m_device->SubmitDrawCalls({dc});

    m_device->DisableRendering();

    m_device->TransitionImageLayout(m_swapchainInfo.images.at(m_swapchainInfo.imageIndex),
                                    VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                                    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                                    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    m_device->EndFrame();

    Present(m_endFrameSemaphore);
}

void Renderer::Shutdown()
{
    DestroySwapchain(m_deviceInfo.device, m_swapchainInfo);
    m_device->OnDestroy();
    ShutdownVulkan();
}

bool Renderer::SetupVulkan()
{
    auto vkbInstanceResult = CreateInstance();
    if (!vkbInstanceResult)
    {
        printf("[ERROR]: %s", vkbInstanceResult.error().message().c_str());
        return false;
    }

    auto vkbInstance      = vkbInstanceResult.value();
    m_deviceInfo.instance = vkbInstance.instance;

#ifdef _DEBUG
    m_debugMessenger = vkbInstance.debug_messenger;
#endif

    m_surface = CreateSurface(m_deviceInfo.instance, m_window);

    auto vkbDeviceResult = CreateDevice(vkbInstance, m_surface);
    if (!vkbDeviceResult)
    {
        printf("[ERROR]: %s", vkbDeviceResult.error().message().c_str());
        return false;
    }

    m_vkbDevice                 = vkbDeviceResult.value();
    m_deviceInfo.device         = m_vkbDevice.device;
    m_deviceInfo.physicalDevice = m_vkbDevice.physical_device;

    m_deviceQueues  = CreateDeviceQueues(m_vkbDevice);
    m_swapchainInfo = CreateSwapchain(m_vkbDevice, m_deviceQueues.at(0), true);

    m_callbacks = {this, &BeginFrameCallback, &EndFrameCallback};

    return true;
}

void Renderer::ShutdownVulkan()
{
    if (m_surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(m_deviceInfo.instance, m_surface, nullptr);

#ifdef _DEBUG
    vkb::destroy_debug_utils_messenger(m_deviceInfo.instance, m_debugMessenger, nullptr);
#endif

    vkDestroyInstance(m_deviceInfo.instance, nullptr);
    m_deviceInfo.instance = VK_NULL_HANDLE;
}

void Renderer::Present(VkSemaphore semaphore)
{
    if (semaphore == VK_NULL_HANDLE)
        return;

    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &m_swapchainInfo.swapchain;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &semaphore;
    presentInfo.pImageIndices      = &m_swapchainInfo.imageIndex;

    auto ok = vkQueuePresentKHR(m_deviceQueues.at(1).queue, &presentInfo);

    if (ok == VK_ERROR_OUT_OF_DATE_KHR || ok == VK_SUBOPTIMAL_KHR)
    {
        // TODO Recreate
    }
}

void Renderer::RecreateSwapchain(bool vsyncEnabled)
{
    // TODO
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
