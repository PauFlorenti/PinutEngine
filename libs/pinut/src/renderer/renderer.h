#pragma once

#include <external/vk-bootstrap/src/VkBootstrap.h>

#include "render_device/device.h"
#include "render_device/renderPipeline.h"

struct GLFWwindow;
namespace RED
{
class Device;
} // namespace RED
namespace Pinut
{
struct SwapchainInfo;
class Renderer final
{
  public:
    explicit Renderer(std::shared_ptr<RED::Device> device,
                      SwapchainInfo*               swapchain,
                      GLFWwindow*                  window,
                      i32                          width  = 720,
                      i32                          height = 1080);
    ~Renderer();
    void Update();

  private:
    static void OnWindowResized(GLFWwindow* window, i32 width, i32 height);

    std::shared_ptr<RED::Device> m_device{nullptr};
    GLFWwindow*                  m_window{nullptr};
    SwapchainInfo*               m_swapchain{nullptr};

    std::unordered_map<std::string, RED::RenderPipeline> m_pipelines;

    i32  m_width{0};
    i32  m_height{0};
    bool bMinimized{false};
};
} // namespace Pinut
