#include "stdafx.h"

#include "VkBootstrap.h"
#include "device.h"
#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"

namespace Pinut
{
void Device::OnCreate(const std::string& applicationName,
                      const std::string& engineName,
                      bool               bGPUValidationEnabled,
                      GLFWwindow*        window)
{
    vkb::InstanceBuilder instanceBuilder;

    auto messaage_severity = //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    auto instance_result = instanceBuilder.set_app_name(applicationName.c_str())
                             .set_debug_messenger_severity(messaage_severity)
                             .request_validation_layers(bGPUValidationEnabled)
                             .use_default_debug_messenger()
                             .require_api_version(1, 3, 0)
                             .build();

    if (!instance_result)
    {
        printf("[ERROR]: %s", instance_result.error().message().c_str());
        return;
    }

    vkb::Instance vkb_instance = instance_result.value();
    m_instance                 = vkb_instance.instance;

#ifdef _DEBUG
    debugMessenger = vkb_instance.debug_messenger;
#endif

    if (glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface) != VK_SUCCESS)
        return;

    VkPhysicalDeviceVulkan12Features features_12{
      .bufferDeviceAddress = true,
    };

    VkPhysicalDeviceVulkan13Features features_13{
      .synchronization2 = true,
      .dynamicRendering = true,
    };

    vkb::PhysicalDeviceSelector gpu_selector{vkb_instance};
    auto                        gpu_result = gpu_selector.set_minimum_version(1, 1)
                        .set_required_features_12(features_12)
                        .set_required_features_13(features_13)
                        .set_surface(m_surface)
                        .select();

    if (!gpu_result)
    {
        printf("[ERROR]: %s", gpu_result.error().message().c_str());
        return;
    }

    auto gpu = gpu_result.value();

    vkb::DeviceBuilder device_builder{gpu};
    vkb::Device        vkb_device = device_builder.build().value();

    m_device                 = vkb_device.device;
    m_physicalDevice         = vkb_device.physical_device;
    graphicsQueue            = vkb_device.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamilyIndex = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
    computeQueue             = vkb_device.get_queue(vkb::QueueType::compute).value();
    computeQueueFamilyIndex  = vkb_device.get_queue_index(vkb::QueueType::compute).value();
    presentQueue             = vkb_device.get_queue(vkb::QueueType::present).value();
    presentQueueFamilyIndex  = vkb_device.get_queue_index(vkb::QueueType::present).value();
}

void Device::OnDestroy()
{
    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    }

    if (m_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

#ifdef _DEBUG
    vkb::destroy_debug_utils_messenger(m_instance, debugMessenger, nullptr);
#endif

    vkDestroyInstance(m_instance, nullptr);
    m_instance = VK_NULL_HANDLE;
}

void Device::WaitIdle() const { vkDeviceWaitIdle(m_device); }
} // namespace Pinut
