#pragma once

//#include "src/core/assetManager.h"
//#include "src/loaders/gltfLoader.h"
//#include "src/loaders/objLoader.h"
//#include "src/renderer/pipelines/forward.h"
//#include "src/renderer/stages/materialManager.h"
//#ifdef _DEBUG
//#include "src/imgui/pinutImgui.h"
//#endif

#include "src/renderer/renderer.h"

struct GLFWwindow;
namespace Pinut
{
class Camera;
class Scene;

struct Mouse
{
    glm::vec2 mousePosition = glm::vec2(0.0f);
    glm::vec2 mouseOffset   = glm::vec2(0.0f);
    f32       wheelSteps    = 0.0f;
};
class Application
{
  public:
    Application(const std::string& name, i32 width = 1280, i32 height = 720);
    ~Application();

    virtual void OnCreate()  = 0;
    virtual void OnDestroy() = 0;
    virtual void OnRender()  = 0;
    virtual void OnUpdate()  = 0;

    static void OnWindowMoved(GLFWwindow* window, int x, int y);
    static void OnMouseMoved(GLFWwindow* window, double xpos, double ypos);
    static void OnMouseWheelRolled(GLFWwindow* window, double xoffset, double yoffset);

    void Init();
    void Run();
    void Shutdown();

    const std::string GetName() const { return m_name; }
    const i32         GetWidth() const { return m_width; }
    const i32         GetHeight() const { return m_height; }
    const f64         GetDeltaTime() const { return m_deltaTime; }

    Camera* GetCamera();

  protected:
    std::string m_name;
    u32         m_width;
    u32         m_height;

    // TODO Shared_ptr
    Camera* m_currentCamera = nullptr;
    Scene*  m_currentScene  = nullptr;

    Mouse m_mouse;

  private:
    bool SetupGlfw();
    void ShutdownGlfw();
    void Update();
    void Render();

    f64 m_deltaTime{0};
    f64 m_lastFrameTime{0};

    std::unique_ptr<Renderer> m_renderer{nullptr};
    GLFWwindow*               m_window{nullptr};

#ifdef _DEBUG
    //PinutImGUI m_imgui;
#endif
};
} // namespace Pinut

i32 Run(std::unique_ptr<Pinut::Application> application);
