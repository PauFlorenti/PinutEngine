#include "stdafx.h"

#include "application.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

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
        }
    }

    application->OnDestroy();
    application->Shutdown();

    return 0;
}

namespace Pinut
{
Application::Application(const std::string& name, i32 width, i32 height)
: _name(name),
  _width(width),
  _height(height)
{
}

void Application::OnWindowMoved(GLFWwindow* window, int x, int y) { printf("Window moved\n"); }

void Application::OnWindowResized(GLFWwindow* window, int width, int height)
{
    printf("Window resized\n");

    auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    assert(app != nullptr);

    if (app->_width == width && app->_height == height)
        return;

    app->_width  = width;
    app->_height = height;

    // app->UpdateDisplay();
}

void Application::Init(GLFWwindow* window)
{
    assert(window);
    if (!window)
        return;

    _window = window;

    // Get framebuffer size
    glfwGetWindowSize(window, &_width, &_height);

    device.OnCreate("Sandbox", "PinutEngine", ENABLE_GPU_VALIDATION_DEFAULT, window);
}

void Application::Shutdown()
{
    device.OnDestroy();
    glfwDestroyWindow(_window);
}
} // namespace Pinut
