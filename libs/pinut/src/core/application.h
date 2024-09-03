#pragma once

#include "src/core/assetManager.h"
#include "src/renderer/commandBufferManager.h"
#include "src/renderer/descriptorSetManager.h"
#include "src/renderer/device.h"
#include "src/renderer/materials/materialManager.h"
#include "src/renderer/pipelines/forward.h"
#include "src/renderer/swapchain.h"
#ifdef _DEBUG
#include "src/imgui/pinutImgui.h"
#endif

struct GLFWwindow;
namespace Pinut
{
class Camera;
class Renderable;
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

    virtual void OnCreate()  = 0;
    virtual void OnDestroy() = 0;
    virtual void OnRender()  = 0;
    virtual void OnUpdate()  = 0;

    static void OnWindowMoved(GLFWwindow* window, int x, int y);
    static void OnWindowResized(GLFWwindow* window, int width, int height);
    static void OnMouseMoved(GLFWwindow* window, double xpos, double ypos);
    static void OnMouseWheelRolled(GLFWwindow* window, double xoffset, double yoffset);

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
    template <typename T>
    std::shared_ptr<T> GetAsset(const std::string& name)
    {
        return m_assetManager.GetAsset<T>(name);
    }

    std::shared_ptr<Renderable> GetRenderable(const std::string& filename, const std::string& name);

    std::shared_ptr<Texture> CreateTextureFromData(const u32          width,
                                                   const u32          height,
                                                   const u32          channels,
                                                   VkFormat           format,
                                                   VkImageUsageFlags  usage,
                                                   void*              data,
                                                   const std::string& name = "");
    std::shared_ptr<Texture> CreateTextureFromFile(const std::string& filename,
                                                   const std::string& name = "");

    std::shared_ptr<MaterialInstance> GetMaterialInstance(const std::string& name,
                                                          MaterialType type = MaterialType::COUNT,
                                                          MaterialData data = {});

    std::string m_name;
    u32         m_width;
    u32         m_height;

    GLFWwindow* m_window{nullptr};

    // TODO Shared_ptr
    Camera* m_currentCamera = nullptr;
    Scene*  m_currentScene  = nullptr;

    Mouse m_mouse;

  private:
    void UpdateDisplay();

    f64 m_deltaTime{0};
    f64 m_lastFrameTime{0};

    Device       m_device;
    Swapchain    m_swapchain;
    AssetManager m_assetManager;

    ForwardPipeline m_forwardPipeline;

#ifdef _DEBUG
    PinutImGUI m_imgui;
#endif

    CommandBufferManager m_commandBufferManager;
};
} // namespace Pinut

i32 Run(Pinut::Application* application);
