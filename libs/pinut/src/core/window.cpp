#include "pch.hpp"

#include <GLFW/glfw3.h>

#include "pinut/core/window.h"

namespace Pinut
{
void WindowDeleter::operator()(GLFWwindow* InWindow) const noexcept
{
    glfwDestroyWindow(InWindow);
    glfwTerminate();
}

Window CreateWindow(u32 InWidth, u32 InHeight, const std::string& InWindowName)
{
    static auto const glfwErrorCallback = [](const i32 InCode, const char* InErrorText)
    {
        std::printf("[GLFW] Error %u: %s", InCode, InErrorText);
    };

    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
    {
        throw std::runtime_error{"[ERROR]: Failed to init GLFW."};
    }

    if (!glfwVulkanSupported())
    {
        throw std::runtime_error{"[ERROR]: Vulkan not supported."};
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    const std::string windowName = "PinutEngine";

    Window window{};
    window.reset(glfwCreateWindow(InWidth, InHeight, InWindowName.c_str(), nullptr, nullptr));

    if (!window)
    {
        glfwTerminate();
        throw std::runtime_error{"[ERROR]: Failed to create GLFW window."};
    }

    return window;
}

VkSurfaceKHR CreateSurface(GLFWwindow* InWindow, const VkInstance InInstance)
{
    VkSurfaceKHR surface{VK_NULL_HANDLE};

    auto ok = glfwCreateWindowSurface(InInstance, InWindow, nullptr, &surface);

    if (ok != VK_SUCCESS || surface == VK_NULL_HANDLE)
    {
        throw std::runtime_error("[ERROR]: Failed to create surface.");
    }

    return surface;
}
} // namespace Pinut
