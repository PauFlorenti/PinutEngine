#include "stdafx.h"

#include "VkBootstrap.h"
#define GLFW_INCLUDE_VULKAN
#include "glfw3.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "device.h"

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

    VmaAllocatorCreateInfo allocator_info{
      .flags          = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = m_physicalDevice,
      .device         = m_device,
      .instance       = m_instance,
    };

    vmaCreateAllocator(&allocator_info, &m_allocator);

    VkCommandPoolCreateInfo commandPoolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags =
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, // | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      .queueFamilyIndex = graphicsQueueFamilyIndex,
    };

    auto ok = vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool);
    assert(ok == VK_SUCCESS);
}

void Device::OnDestroy()
{
    if (m_commandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);

    vmaDestroyAllocator(m_allocator);

    if (m_surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

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

VkCommandBuffer Device::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo info{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = m_commandPool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
    };

    VkCommandBuffer cmd;
    assert(vkAllocateCommandBuffers(m_device, &info, &cmd) == VK_SUCCESS);

    VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    assert(vkBeginCommandBuffer(cmd, &beginInfo) == VK_SUCCESS);
    return cmd;
}

void Device::FlushCommandBuffer(VkCommandBuffer cmd) const
{
    vkEndCommandBuffer(cmd);

    VkSubmitInfo info{.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                      .pNext              = nullptr,
                      .commandBufferCount = 1,
                      .pCommandBuffers    = &cmd};

    vkQueueSubmit(graphicsQueue, 1, &info, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &cmd);
}

void Device::WaitIdle() const { vkDeviceWaitIdle(m_device); }
} // namespace Pinut
