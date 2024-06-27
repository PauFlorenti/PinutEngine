#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Pinut
{
class Device;

class Swapchain
{
  public:
    Swapchain() = default;

    void OnCreate(Device* device, u32 backbufferCount, GLFWwindow* window);
    void OnDestroy();

    void OnCreateWindowDependantResources(u32 width, u32 height);
    void OnDestroyWindowDependantResources();

    u32  WaitForSwapchain();
    void Present();

    void GetSyncObjects(VkSemaphore* outImageAvailableSemaphores,
                        VkSemaphore* outRenderFinishedSemaphores,
                        VkFence*     outFences);

  private:
    Device*     m_device{nullptr};
    GLFWwindow* m_window{nullptr};

    VkSwapchainKHR           m_swapchain{VK_NULL_HANDLE};
    VkSurfaceFormatKHR       m_swapchainFormat{};
    VkSurfaceCapabilitiesKHR m_capabilities{};

    u32 m_backbufferCount{0}, m_imageIndex{0}, m_frameIndex{0};
    u32 m_width{0}, m_height{0};

    std::vector<VkImage>       m_images;
    std::vector<VkImageView>   m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers;

    VkFence                  m_fence{VK_NULL_HANDLE};
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
};
} // namespace Pinut
