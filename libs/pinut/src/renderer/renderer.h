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
class Renderer final
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

    using DeviceQueues = std::array<QueueInfo, 3>;

    struct SwapchainInfo
    {
        VkSwapchainKHR     swapchain{VK_NULL_HANDLE};
        VkSurfaceFormatKHR surfaceFormat{};
        VkExtent2D         surfaceExtent{};

        std::vector<VkImage>     images;
        std::vector<VkImageView> imageViews;
        u32                      imageIndex{0};
        bool                     vsyncEnabled{true};
    };

    struct DeviceCallbacks
    {
        void*                                   context;
        std::function<void(void*, VkSemaphore)> BeginFrame_fn;
        std::function<void(void*, VkSemaphore)> EndFrame_fn;
    };

  public:
    Renderer(GLFWwindow* window, i32 width = 720, i32 height = 1080);
    void Update();
    void Shutdown();

  private:
    static void                       OnWindowResized(GLFWwindow* window, i32 width, i32 height);
    static vkb::Result<vkb::Instance> CreateInstance();
    static VkSurfaceKHR               CreateSurface(const VkInstance& instance, GLFWwindow* window);
    static vkb::Result<vkb::Device>   CreateDevice(const vkb::Instance& vkbInstance,
                                                   const VkSurfaceKHR&  surface);
    static DeviceQueues               CreateDeviceQueues(const vkb::Device& vkbDevice);
    static SwapchainInfo              CreateSwapchain(const vkb::Device& vkbDevice,
                                                      const QueueInfo&   queueInfos,
                                                      bool               vsyncEnabled);

    static void DestroySwapchain(VkDevice& device, const SwapchainInfo& swapchainInfo);

    static void BeginFrameCallback(void* context, VkSemaphore imageAvailableSemaphore);
    static void EndFrameCallback(void* context, VkSemaphore renderFinishedSemaphore);

    bool SetupVulkan();
    void ShutdownVulkan();

    void Present(VkSemaphore semaphore);
    void RecreateSwapchain(bool vsyncEnabled);

    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

    vkb::Device   m_vkbDevice;
    DeviceQueues  m_deviceQueues;
    DeviceInfo    m_deviceInfo;
    SwapchainInfo m_swapchainInfo;

    GLFWwindow*                  m_window{nullptr};
    std::unique_ptr<RED::Device> m_device{nullptr};

    VkSemaphore m_endFrameSemaphore{VK_NULL_HANDLE};

    DeviceCallbacks m_callbacks;

    std::unordered_map<std::string, RED::RenderPipeline> m_pipelines;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
#endif

    i32  m_width{0};
    i32  m_height{0};
    bool bMinimized{false};
};
} // namespace Pinut
