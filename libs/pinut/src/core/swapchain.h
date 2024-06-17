#pragma once

#include "device.h"

struct GLFWwindow;

namespace Pinut
{
    class Texture;
    class Swapchain
    {
    public:
        Swapchain() = default;

        void OnCreate(Device *device, const uint32_t &numberBackBuffers, GLFWwindow *window);
        void OnDestroy();

        void OnCreateWindowDependantResources(uint32_t inWidth, uint32_t inHeihgt);
        void OnDestroyWindowDependantResources();

        VkResult Present();
        uint32_t WaitForSwapchain();

        bool GetSyncObjects(VkSemaphore *outImageAvailableSemaphores, VkSemaphore *outRenderFinishedSemaphores, VkFence *outFences);
        uint32_t GetImageIndex() const { return imageIndex; }
        uint32_t GetFrameIndex() const { return frameIndex; }
        VkImage GetCurrentImage() const { return images[imageIndex]; }
        VkImage GetImage(int i) const { return images[i]; }
        VkImageView GetCurrentImageView() const { return imageViews[imageIndex]; }
        VkImageView GetImageView(int i) const { return imageViews[i]; }

    private:
        Device *deviceHandle{nullptr};
        GLFWwindow *windowHandle{nullptr};

        VkSwapchainKHR swapchain{VK_NULL_HANDLE};
        VkSurfaceFormatKHR swapchainFormat;

        VkQueue presentQueue;

        std::vector<VkImage> images;
        std::vector<VkImageView> imageViews;
        std::vector<VkFramebuffer> framebuffers;

        std::vector<VkFence> fences;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;

        uint32_t width{0};
        uint32_t height{0};

        uint32_t imageIndex{0};
        uint32_t backbufferCount{2};
        uint32_t frameIndex{0};
    };
}
