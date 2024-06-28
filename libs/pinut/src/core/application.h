#pragma once

#include "src/renderer/device.h"
#include "src/renderer/swapchain.h"

struct GLFWwindow;
namespace Pinut
{
class Application
{
  public:
    Application(const std::string& name, i32 width = 1280, i32 height = 720);

    virtual void OnCreate()  = 0;
    virtual void OnDestroy() = 0;
    virtual void OnRender()  = 0;
    virtual void OnUpdate()  = 0;

    const std::string GetName() const { return m_name; }
    const i32         GetWidth() const { return m_width; }
    const i32         GetHeight() const { return m_height; }

    static void OnWindowMoved(GLFWwindow* window, int x, int y);
    static void OnWindowResized(GLFWwindow* window, int width, int height);

    void Init(GLFWwindow* window);
    void Shutdown();
    void Render();

  protected:
    std::string m_name;
    u32         m_width;
    u32         m_height;

    GLFWwindow* m_window{nullptr};

  private:
    void UpdateDisplay();

    Device    m_device;
    Swapchain m_swapchain;

    // Temporal
    VkCommandBuffer  cmds[3] = {};
    VkCommandPool    commandPool{VK_NULL_HANDLE};
    VkPipeline       pipeline{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
};
} // namespace Pinut

i32 Run(Pinut::Application* application);
