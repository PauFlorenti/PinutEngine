#pragma once

#include "src/renderer/device.h"

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

    const std::string GetName() const { return _name; }
    const i32         GetWidth() const { return _width; }
    const i32         GetHeight() const { return _height; }

    static void OnWindowMoved(GLFWwindow* window, int x, int y);
    static void OnWindowResized(GLFWwindow* window, int inWidth, int inHeight);

    void Init(GLFWwindow* window);
    void Shutdown();

  protected:
    std::string _name;
    i32         _width;
    i32         _height;

    GLFWwindow* _window{nullptr};

    Device device;
};
} // namespace Pinut

i32 Run(Pinut::Application* application);
