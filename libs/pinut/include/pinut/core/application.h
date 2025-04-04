#pragma once

#include "pinut/core/assetManager.h"
#include "pinut/core/window.h"
#include "pinut/renderer/renderer.h"
#include "pinut/renderer/swapchain.h"
#ifdef _DEBUG
#include "pinut/imgui/pinutImgui.h"
#endif

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
    struct DeviceInfo
    {
        VkInstance       instance;
        VkDevice         device;
        VkPhysicalDevice physicalDevice;
    };

    struct QueueInfo
    {
        VkQueue queue{VK_NULL_HANDLE};
        u32     index;
    };

    struct SwapchainState
    {
        VkSwapchainKHR swapchain;
        VkImage        swapchainImage;
        VkImageView    swapchainImageView;
        u32            swapchainImageIndex;
        VkSemaphore    endFrameSemaphore;
    };

    using DeviceQueues = std::array<QueueInfo, 3>;

    struct DeviceCallbacks
    {
        void*                                   context;
        std::function<void(void*, VkSemaphore)> BeginFrame_fn;
        std::function<void(void*, VkSemaphore)> EndFrame_fn;
        std::function<SwapchainState()>         GetSwapchainState_fn;
    };

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
    void RecreateSwapchain(bool bVsync);

    const std::string GetName() const { return m_name; }
    const i32         GetWidth() const { return m_width; }
    const i32         GetHeight() const { return m_height; }
    const f64         GetDeltaTime() const { return m_deltaTime; }

    Camera* GetCamera();

  protected:
    AssetManager m_assetManager;

    std::string m_name;
    u32         m_width;
    u32         m_height;

    // TODO Shared_ptr
    Camera* m_currentCamera{nullptr};
    Scene*  m_currentScene{nullptr};
    Window  m_window{nullptr};

    Mouse m_mouse;

  private:
    static vkb::Result<vkb::Instance> CreateInstance();
    static vkb::Result<vkb::Device>   CreateDevice(const vkb::Instance& vkbInstance,
                                                   const VkSurfaceKHR&  surface);
    static DeviceQueues               CreateDeviceQueues(const vkb::Device& vkbDevice);
    static SwapchainInfo              CreateSwapchain(const vkb::Device& vkbDevice,
                                                      const QueueInfo&   queueInfos,
                                                      bool               vsyncEnabled);
    static void DestroySwapchain(VkDevice& device, const SwapchainInfo& swapchainInfo);

    static void BeginFrameCallback(void* context, VkSemaphore imageAvailableSemaphore);
    static void EndFrameCallback(void* context, VkSemaphore renderFinishedSemaphore);

    static void OnWindowResized(GLFWwindow* window, i32 width, i32 height);

    bool SetupVulkan();
    void ShutdownVulkan();

    void Update();
    void Render();
    void Present();

    f64 m_deltaTime{0};
    f64 m_lastFrameTime{0};

    ViewportData m_viewport;

    // RENDERING
    std::unique_ptr<Renderer> m_renderer{nullptr};
    VkSemaphore               m_endFrameSemaphore{VK_NULL_HANDLE};

    VkSurfaceKHR    m_surface{VK_NULL_HANDLE};
    vkb::Device     m_vkbDevice;
    DeviceQueues    m_deviceQueues;
    DeviceInfo      m_deviceInfo;
    SwapchainInfo   m_swapchainInfo;
    DeviceCallbacks m_callbacks;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
#endif
};
} // namespace Pinut

i32 Run(std::unique_ptr<Pinut::Application> application);
