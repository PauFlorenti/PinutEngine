#pragma once

struct GLFWwindow;

namespace Pinut
{
struct WindowDeleter
{
    void operator()(GLFWwindow* InWindow) const noexcept;
};

using Window = std::unique_ptr<GLFWwindow, WindowDeleter>;

Window CreateWindow(u32 InWidth, u32 InHeight, const std::string& InWindowName);

VkSurfaceKHR CreateSurface(GLFWwindow* InWindow, const VkInstance InInstance);

} // namespace Pinut
