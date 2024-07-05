#pragma once

#include "src/renderer/commandBufferManager.h"
#include "src/renderer/device.h"
#include "src/renderer/pipelines/forward.h"
#include "src/renderer/swapchain.h"

struct GLFWwindow;
namespace Pinut
{
class Camera;
class Scene;
class Application
{
  public:
    Application(const std::string& name, i32 width = 1280, i32 height = 720);

    virtual void OnCreate()  = 0;
    virtual void OnDestroy() = 0;
    virtual void OnRender()  = 0;
    virtual void OnUpdate()  = 0;

    static void OnWindowMoved(GLFWwindow* window, int x, int y);
    static void OnWindowResized(GLFWwindow* window, int width, int height);

    void Init(GLFWwindow* window);
    void Shutdown();
    void Update();
    void Render();

    const std::string GetName() const { return m_name; }
    const i32         GetWidth() const { return m_width; }
    const i32         GetHeight() const { return m_height; }
    const f64         GetDeltaTime() const { return m_deltaTime; }

    Camera* GetCamera();

  protected:
    std::string m_name;
    u32         m_width;
    u32         m_height;

    GLFWwindow* m_window{nullptr};

    Camera* m_currentCamera = nullptr;
    Scene*  m_currentScene  = nullptr;

  private:
    void UpdateDisplay();

    f64 m_deltaTime{0};
    f64 m_lastFrameTime{0};

    Device          m_device;
    Swapchain       m_swapchain;
    ForwardPipeline m_forwardPipeline;

    CommandBufferManager m_commandBufferManager;
};
} // namespace Pinut

i32 Run(Pinut::Application* application);
