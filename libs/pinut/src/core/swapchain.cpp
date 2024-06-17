#include "stdafx.h"

#include "swapchain.h"

#include "VkBootstrap.h"

#include "texture.h"

namespace Pinut
{
    void Swapchain::OnCreate(Device *inDevice, const uint32_t &numberBackBuffers, GLFWwindow *inWindow)
    {
        deviceHandle = inDevice;
        backbufferCount = numberBackBuffers;
        windowHandle = inWindow;

        presentQueue = deviceHandle->GetPresentQueue();

        // Set default safe formats at start.
        VkSurfaceFormatKHR surfaceFormat;
        surfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
        surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchainFormat = surfaceFormat;

        fences.resize(backbufferCount);
        imageAvailableSemaphores.resize(backbufferCount);
        renderFinishedSemaphores.resize(backbufferCount);

        VkDevice device = deviceHandle->GetDevice();

        VkResult ok;
        for (uint32_t i = 0; i < backbufferCount; ++i)
        {
            VkFenceCreateInfo fenceInfo{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT,
            };

            ok = vkCreateFence(device, &fenceInfo, nullptr, &fences[i]);
            assert(ok == VK_SUCCESS);

            VkSemaphoreCreateInfo semaphoreInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            };

            ok = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
            assert(ok == VK_SUCCESS);

            ok = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
            assert(ok == VK_SUCCESS);
        }
    }

    void Swapchain::OnDestroy()
    {
        auto device = deviceHandle->GetDevice();
        for (uint32_t i = 0; i < backbufferCount; ++i)
        {
            vkDestroyFence(device, fences[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        }

        OnDestroyWindowDependantResources();
    }

    void Swapchain::OnCreateWindowDependantResources(uint32_t inWidth, uint32_t inHeight)
    {
        width = inWidth;
        height = inHeight;

        vkb::SwapchainBuilder swapchainBuilder{deviceHandle->GetPhysicalDevice(), deviceHandle->GetDevice(), deviceHandle->GetSurface()};
        auto result = swapchainBuilder
                          .set_desired_extent(inWidth, inHeight)
                          .set_desired_format(swapchainFormat)
                          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                          .build();

        if (!result || !result.has_value())
            assert(false);

        auto resultValue = result.value();
        swapchain = resultValue.swapchain;
        images = resultValue.get_images().value();
        imageViews = resultValue.get_image_views().value();
    }

    void Swapchain::OnDestroyWindowDependantResources()
    {
        for (auto &view : imageViews)
        {
            vkDestroyImageView(deviceHandle->GetDevice(), view, nullptr);
        }

        vkDestroySwapchainKHR(deviceHandle->GetDevice(), swapchain, nullptr);
    }

    VkResult Swapchain::Present()
    {

        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphores[frameIndex],
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &imageIndex};

        frameIndex = ++frameIndex % backbufferCount;
        deviceHandle->SetFrameIndex(frameIndex);

        return vkQueuePresentKHR(presentQueue, &presentInfo);
    }

    uint32_t Swapchain::WaitForSwapchain()
    {
        vkAcquireNextImageKHR(deviceHandle->GetDevice(), swapchain, UINT64_MAX, imageAvailableSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex);

        vkWaitForFences(deviceHandle->GetDevice(), 1, &fences[frameIndex], VK_TRUE, UINT64_MAX);
        vkResetFences(deviceHandle->GetDevice(), 1, &fences[frameIndex]);

        return imageIndex;
    }

    bool Swapchain::GetSyncObjects(VkSemaphore *outImageAvailableSemaphores, VkSemaphore *outRenderFinishedSemaphores, VkFence *outFences)
    {
        *outImageAvailableSemaphores = imageAvailableSemaphores[frameIndex];
        *outRenderFinishedSemaphores = renderFinishedSemaphores[frameIndex];
        *outFences = fences[frameIndex];

        return true;
    }
}
