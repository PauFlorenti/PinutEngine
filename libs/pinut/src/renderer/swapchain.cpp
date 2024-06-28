#include "stdafx.h"

#include "VkBootstrap.h"
#include "device.h"
#include "swapchain.h"

namespace Pinut
{
void Swapchain::OnCreate(Device* device, u32 backbufferCount, GLFWwindow* window)
{
    m_backbufferCount = backbufferCount;
    m_device          = device;
    m_window          = window;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->GetPhysicalDevice(),
                                              m_device->GetSurface(),
                                              &m_capabilities);

    // Set default safe formats at start.
    VkSurfaceFormatKHR surfaceFormat;
    surfaceFormat.format     = VK_FORMAT_B8G8R8A8_UNORM;
    surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    m_swapchainFormat        = surfaceFormat;

    m_imageAvailableSemaphores.resize(m_backbufferCount);
    m_renderFinishedSemaphores.resize(m_backbufferCount);

    auto deviceHandle = m_device->GetDevice();

    VkResult          ok;
    VkFenceCreateInfo fenceInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    ok = vkCreateFence(deviceHandle, &fenceInfo, nullptr, &m_fence);
    assert(ok == VK_SUCCESS);

    for (uint32_t i = 0; i < m_backbufferCount; ++i)
    {
        VkSemaphoreCreateInfo semaphoreInfo = {
          .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        ok =
          vkCreateSemaphore(deviceHandle, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]);
        assert(ok == VK_SUCCESS);

        ok =
          vkCreateSemaphore(deviceHandle, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]);
        assert(ok == VK_SUCCESS);
    }
}

void Swapchain::OnDestroy()
{
    const auto device = m_device->GetDevice();

    vkDestroyFence(device, m_fence, nullptr);

    for (u32 i = 0; i < m_backbufferCount; ++i)
    {
        vkDestroySemaphore(device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, m_renderFinishedSemaphores[i], nullptr);
    }

    OnDestroyWindowDependantResources();
}

void Swapchain::OnCreateWindowDependantResources(uint32_t width, uint32_t height)
{
    m_width  = width;
    m_height = height;

    vkb::SwapchainBuilder swapchainBuilder{m_device->GetPhysicalDevice(),
                                           m_device->GetDevice(),
                                           m_device->GetSurface()};
    auto                  result = swapchainBuilder.set_desired_extent(m_width, m_height)
                    .set_desired_format(m_swapchainFormat)
                    .add_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
                    .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                    .build();

    if (!result || !result.has_value())
        assert(false);

    auto resultValue = result.value();
    m_swapchain      = resultValue.swapchain;
    m_images         = resultValue.get_images().value();
    m_imageViews     = resultValue.get_image_views().value();
}

void Swapchain::OnDestroyWindowDependantResources()
{
    for (auto& view : m_imageViews)
    {
        vkDestroyImageView(m_device->GetDevice(), view, nullptr);
    }

    if (m_swapchain)
        vkDestroySwapchainKHR(m_device->GetDevice(), m_swapchain, nullptr);
}

u32 Swapchain::WaitForSwapchain()
{
    const auto& device = m_device->GetDevice();
    vkAcquireNextImageKHR(device,
                          m_swapchain,
                          UINT64_MAX,
                          m_imageAvailableSemaphores[m_frameIndex],
                          VK_NULL_HANDLE,
                          &m_imageIndex);

    vkWaitForFences(device, 1, &m_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &m_fence);

    return m_imageIndex;
}

void Swapchain::Present()
{
    VkPresentInfoKHR info{
      .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores    = &m_renderFinishedSemaphores[m_frameIndex],
      .swapchainCount     = 1,
      .pSwapchains        = &m_swapchain,
      .pImageIndices      = &m_imageIndex,
    };

    auto ok = vkQueuePresentKHR(m_device->GetPresentQueue(), &info);
    assert(ok == VK_SUCCESS);

    m_frameIndex = ++m_frameIndex % m_backbufferCount;
}

void Swapchain::GetSyncObjects(VkSemaphore* outImageAvailableSemaphores,
                               VkSemaphore* outRenderFinishedSemaphores,
                               VkFence*     outFences)
{
    *outImageAvailableSemaphores = m_imageAvailableSemaphores[m_frameIndex];
    *outRenderFinishedSemaphores = m_renderFinishedSemaphores[m_frameIndex];
    *outFences                   = m_fence;
}

VkImage Swapchain::GetCurrentImage() const { return m_images.at(m_imageIndex); }

VkImageView Swapchain::GetCurrentImageView() const { return m_imageViews.at(m_imageIndex); }
} // namespace Pinut
