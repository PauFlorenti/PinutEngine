#include "pch.hpp"

#include <GLFW/glfw3.h>
#include <render_device/device.h>

#include "pinut/assets/mesh.h"
#include "pinut/assets/texture.h"
#include "pinut/components/meshComponent.h"
#include "pinut/components/renderComponent.h"
#include "pinut/components/transformComponent.h"
#include "pinut/core/application.h"
#include "pinut/core/camera.h"
#include "pinut/core/scene.h"
#include "pinut/renderer/primitives.h"
#include "pinut/renderer/renderer.h"

#if _DEBUG
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = true;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = true;
#else // RELEASE
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = false;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = false;
#endif

static bool bMinimized{false};
static bool bResized{true};

i32 Run(std::unique_ptr<Pinut::Application> application)
{
    assert(application);
    if (!application)
        return -1;

    application->OnCreate();
    application->Run();
    application->OnDestroy();

    return 0;
}

namespace Pinut
{
vkb::Result<vkb::Instance> Application::CreateInstance()
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

vkb::Result<vkb::Device> Application::CreateDevice(const vkb::Instance& vkbInstance,
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

Application::DeviceQueues Application::CreateDeviceQueues(const vkb::Device& vkbDevice)
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

SwapchainInfo Application::CreateSwapchain(const vkb::Device& vkbDevice,
                                           const QueueInfo&   queueInfos,
                                           bool               vsyncEnabled)
{
    vkb::SwapchainBuilder builder{vkbDevice};
    auto                  result =
      builder
        .set_desired_present_mode(vsyncEnabled ? VK_PRESENT_MODE_FIFO_KHR :
                                                 VK_PRESENT_MODE_IMMEDIATE_KHR)
        .set_desired_format({VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
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

void Application::DestroySwapchain(VkDevice& device, const SwapchainInfo& swapchainInfo)
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

Application::Application(const std::string& name, i32 width, i32 height)
: m_name(name),
  m_width(width),
  m_height(height)
{
    const std::string windowName = "PinutEngine";
    m_window                     = CreateWindow(m_width, m_height, windowName);

    glfwSetWindowUserPointer(m_window.get(), this);
    glfwSetWindowSizeCallback(m_window.get(), &Application::OnWindowResized);
    glfwSetCursorPosCallback(m_window.get(), &Application::OnMouseMoved);

    if (!SetupVulkan())
        return;

    auto device = RED::Device::Create(&m_deviceInfo, m_deviceQueues.data(), &m_callbacks);

#ifdef _DEBUG
    auto imgui =
      std::make_unique<PinutImGUI>(&m_deviceInfo,
                                   &m_deviceQueues.at(static_cast<u32>(RED::QueueType::GRAPHICS)),
                                   m_window.get(),
                                   m_width,
                                   m_height);
    m_renderer = std::make_unique<Renderer>(std::move(device), &m_swapchainInfo, std::move(imgui));
#else
    m_renderer = std::make_unique<Renderer>(std::move(device), &m_swapchainInfo);
#endif
}

Application::~Application()
{
    if (m_currentScene)
        m_currentScene->ClearScene();

    DestroySwapchain(m_deviceInfo.device, m_swapchainInfo);
    m_renderer.reset();
    ShutdownVulkan();
}

void Application::OnWindowMoved(GLFWwindow* window, int x, int y)
{
    printf("Window moved to x: %d y: %d\n", x, y);
}

void Application::OnMouseMoved(GLFWwindow* window, double xpos, double ypos)
{
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    assert(app != nullptr);

    app->m_mouse.mouseOffset =
      glm::vec2(xpos - app->m_mouse.mousePosition.x, ypos - app->m_mouse.mousePosition.y);
    app->m_mouse.mousePosition = glm::vec2(xpos, ypos);
}

void Application::OnMouseWheelRolled(GLFWwindow* window, double xoffset, double yoffset)
{
    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    assert(app != nullptr);

    app->m_mouse.wheelSteps += static_cast<f32>(yoffset);
}

void Application::Init() {}

void Application::Run()
{
    while (!glfwWindowShouldClose(m_window.get()))
    {
        // Check if we should close
        if (glfwGetKey(m_window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(m_window.get(), true);
        }

        glfwPollEvents();

        if (!bMinimized)
        {
            Update();
            OnUpdate();
            OnRender();
            Render();
            Present();
        }
    }
}

void Application::RecreateSwapchain(bool bSync)
{
    vkDeviceWaitIdle(m_deviceInfo.device);
    DestroySwapchain(m_deviceInfo.device, m_swapchainInfo);
    m_swapchainInfo =
      CreateSwapchain(m_vkbDevice, m_deviceQueues.at(0), m_swapchainInfo.vsyncEnabled);
}

void Application::Update()
{
    static auto startTime = glfwGetTime();

    auto currentTime = glfwGetTime();
    m_deltaTime      = currentTime - m_lastFrameTime;
    m_lastFrameTime  = currentTime;

    m_viewport.width                     = m_width;
    m_viewport.height                    = m_height;
    m_viewport.cameraData.view           = m_currentCamera->View();
    m_viewport.cameraData.projection     = m_currentCamera->Projection();
    m_viewport.cameraData.cameraPosition = m_currentCamera->Position();
}

void Application::Render() { m_renderer->Render(m_currentScene->Registry(), m_viewport, bResized); }

void Application::Present()
{
    if (m_endFrameSemaphore == VK_NULL_HANDLE)
        return;

    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &m_swapchainInfo.swapchain;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &m_endFrameSemaphore;
    presentInfo.pImageIndices      = &m_swapchainInfo.imageIndex;

    auto ok = vkQueuePresentKHR(m_deviceQueues.at(1).queue, &presentInfo);

    if (ok == VK_ERROR_OUT_OF_DATE_KHR || ok == VK_SUBOPTIMAL_KHR || bResized)
    {
        // TODO Vsync should be given by the application.
        RecreateSwapchain(true);
        bResized = false;
    }
}

Camera* Application::GetCamera()
{
    if (!m_currentCamera)
        m_currentCamera = new Camera();

    return m_currentCamera;
}

void Application::OnWindowResized(GLFWwindow* window, i32 width, i32 height)
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

    bResized   = true;
    bMinimized = false;

    app->m_width  = width;
    app->m_height = height;
}

bool Application::SetupVulkan()
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

    m_surface = CreateSurface(m_window.get(), m_deviceInfo.instance);

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

    auto BeginFrameCallback = [](void* context, VkSemaphore imageAvailableSemaphore)
    {
        auto app = reinterpret_cast<Application*>(context);

        if (bResized)
        {
            app->RecreateSwapchain(app->m_swapchainInfo.vsyncEnabled);
            bResized = false;
        }

        auto ok = vkAcquireNextImageKHR(app->m_deviceInfo.device,
                                        app->m_swapchainInfo.swapchain,
                                        UINT64_MAX,
                                        imageAvailableSemaphore,
                                        VK_NULL_HANDLE,
                                        &app->m_swapchainInfo.imageIndex);

        if (ok == VK_ERROR_OUT_OF_DATE_KHR)
        {
            app->RecreateSwapchain(app->m_swapchainInfo.vsyncEnabled);

            vkAcquireNextImageKHR(app->m_deviceInfo.device,
                                  app->m_swapchainInfo.swapchain,
                                  UINT64_MAX,
                                  imageAvailableSemaphore,
                                  VK_NULL_HANDLE,
                                  &app->m_swapchainInfo.imageIndex);
        }
    };

    auto EndFrameCallback = [](void* context, VkSemaphore renderFinishedSemaphore)
    {
        auto app                 = reinterpret_cast<Application*>(context);
        app->m_endFrameSemaphore = renderFinishedSemaphore;
    };

    auto GetSwapchainStateCallback = [&]()
    {
        SwapchainState state{};
        state.endFrameSemaphore   = m_endFrameSemaphore;
        state.swapchain           = m_swapchainInfo.swapchain;
        state.swapchainImage      = m_swapchainInfo.images.at(m_swapchainInfo.imageIndex);
        state.swapchainImageView  = m_swapchainInfo.imageViews.at(m_swapchainInfo.imageIndex);
        state.swapchainImageIndex = m_swapchainInfo.imageIndex;
        return state;
    };

    m_callbacks = {this,
                   std::move(BeginFrameCallback),
                   std::move(EndFrameCallback),
                   std::move(GetSwapchainStateCallback)};

    return true;
}

void Application::ShutdownVulkan()
{
    if (m_surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(m_deviceInfo.instance, m_surface, nullptr);

#ifdef _DEBUG
    vkb::destroy_debug_utils_messenger(m_deviceInfo.instance, m_debugMessenger, nullptr);
#endif

    vkDestroyInstance(m_deviceInfo.instance, nullptr);
    m_deviceInfo.instance = VK_NULL_HANDLE;
}
} // namespace Pinut
