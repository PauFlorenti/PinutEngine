// Some comment

#include "stdafx.h"

#include "application.h"

#if _DEBUG
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = true;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = true;
#else // RELEASE
static constexpr bool ENABLE_CPU_VALIDATION_DEFAULT = false;
static constexpr bool ENABLE_GPU_VALIDATION_DEFAULT = false;
#endif

static bool bMinimized = false;

int32_t Run(Pinut::Application *application)
{
    // TODO Init logging

    assert(application);
    if (!application)
        return -1;

    // Init window
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    auto window = glfwCreateWindow(application->GetWidth(), application->GetHeight(), application->GetName().c_str(), nullptr, nullptr);

    if (!window)
        return -1;

    application->DeviceInit(window);

    glfwSetWindowUserPointer(window, application);

    glfwSetWindowSizeCallback(window, &Pinut::Application::OnWindowResized);
    glfwSetWindowPosCallback(window, &Pinut::Application::OnWindowMoved);

    application->OnCreate();

    application->UpdateDisplay();

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
        }
    }

    application->WaitIdle();

    application->OnDestroy();

    application->DeviceShutdown();

    return 0;
}

namespace Pinut
{
    Application::Application(const char *inName, int32_t inWidth, int32_t inHeight) : name(inName),
                                                                                      width(inWidth),
                                                                                      height(inHeight),

                                                                                      deltaTime(0),
                                                                                      lastFrameTime(0),

                                                                                      windowHandle(nullptr),

                                                                                      bCPUValidationEnabled(ENABLE_CPU_VALIDATION_DEFAULT),
                                                                                      bGPUValidationEnabled(ENABLE_GPU_VALIDATION_DEFAULT)
    {
    }

    void Application::DeviceInit(GLFWwindow *window)
    {
        windowHandle = std::move(window);

        // Get framebuffer size without
        glfwGetWindowSize(windowHandle, &width, &height);

        // Create Device
        device.OnCreate(name.c_str(), "PinutEngine", bCPUValidationEnabled, bGPUValidationEnabled, windowHandle);

        // Create Swapchain
        swapchain.OnCreate(&device, 2, windowHandle);
    }

    void Application::DeviceShutdown()
    {
        WaitIdle();

        swapchain.OnDestroy();
        device.OnDestroy();
    }

    void Application::WaitIdle()
    {
        vkDeviceWaitIdle(device.GetDevice());
    }

    void Application::OnUpdate()
    {
        static auto startTime = glfwGetTime();

        auto currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
    }

    void Application::UpdateDisplay()
    {
        WaitIdle();

        swapchain.OnDestroyWindowDependantResources();
        swapchain.OnCreateWindowDependantResources(width, height);

        OnUpdateDisplay();
    }

    void Application::OnWindowMoved(GLFWwindow *window, int x, int y)
    {
        printf("Window moved\n");
    }

    void Application::OnWindowResized(GLFWwindow *window, int inWidth, int inHeight)
    {
        printf("Window resized\n");

        auto app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        assert(app != nullptr);

        if (app->width == inWidth && app->height == inHeight)
            return;

        app->width = inWidth;
        app->height = inHeight;

        app->UpdateDisplay();
    }
}
