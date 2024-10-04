#pragma once

#include <external/vk-bootstrap/src/VkBootstrap.h>
#include <render_device/src/device.h>

struct GLFWwindow;
namespace vulkan
{
class Device;
class Swapchain;
} // namespace vulkan
namespace Pinut
{
class Renderer final
{
    using DeviceQueues = std::array<vulkan::QueueInfo, 3>;

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
    static SwapchainInfo              CreateSwapchain(const vkb::Device&       vkbDevice,
                                                      const vulkan::QueueInfo& queueInfos,
                                                      bool                     vsyncEnabled);

    static void DestroySwapchain(VkDevice& device, const SwapchainInfo& swapchainInfo);

    static void BeginFrameCallback(void* context, VkSemaphore imageAvailableSemaphore);
    static void EndFrameCallback(void* context, VkSemaphore renderFinishedSemaphore);

    bool SetupVulkan();
    void ShutdownVulkan();

    void Present(VkSemaphore semaphore);
    void RecreateSwapchain(bool vsyncEnabled);

    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

    vkb::Device        m_vkbDevice;
    DeviceQueues       m_deviceQueues;
    vulkan::DeviceInfo m_deviceInfo;
    SwapchainInfo      m_swapchainInfo;

    GLFWwindow*                     m_window{nullptr};
    std::unique_ptr<vulkan::Device> m_device{nullptr};

    VkSemaphore m_endFrameSemaphore{VK_NULL_HANDLE};

    vulkan::DeviceCallbacks m_callbacks;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT m_debugMessenger{nullptr};
#endif

    i32  m_width{0};
    i32  m_height{0};
    bool bMinimized{false};
};
} // namespace Pinut
