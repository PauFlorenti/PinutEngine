// Some comments
#include "stdafx.h"

#include "device.h"

#include "VkBootstrap.h"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace Pinut
{
    void Device::OnCreate(const char *applicationName, const char *engineName, bool bCPUValidationEnabled, bool bGPUValidationEnabled, GLFWwindow *windowHandle)
    {
        vkb::InstanceBuilder instance_builder;

        auto instance_result = instance_builder
                                   .set_app_name(applicationName)
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
        instance = vkb_instance.instance;

#ifdef _DEBUG
        debugMessenger = vkb_instance.debug_messenger;
#endif

        if (glfwCreateWindowSurface(instance, windowHandle, nullptr, &surface) != VK_SUCCESS)
            return;

        VkPhysicalDeviceVulkan12Features features_12{
            .bufferDeviceAddress = true,
        };

        VkPhysicalDeviceVulkan13Features features_13{
            .synchronization2 = true,
            .dynamicRendering = true,
        };

        vkb::PhysicalDeviceSelector gpu_selector{vkb_instance};
        auto gpu_result = gpu_selector
                              .set_minimum_version(1, 1)
                              .set_required_features_12(features_12)
                              .set_required_features_13(features_13)
                              .set_surface(surface)
                              .select();

        if (!gpu_result)
        {
            printf("[ERROR]: %s", gpu_result.error().message().c_str());
            return;
        }

        auto gpu = gpu_result.value();

        vkb::DeviceBuilder device_builder{gpu};
        vkb::Device vkb_device = device_builder.build().value();

        device = vkb_device.device;
        physicalDevice = vkb_device.physical_device;
        graphicsQueue = vkb_device.get_queue(vkb::QueueType::graphics).value();
        graphicsQueueFamilyIndex = vkb_device.get_queue_index(vkb::QueueType::graphics).value();
        computeQueue = vkb_device.get_queue(vkb::QueueType::compute).value();
        computeQueueFamilyIndex = vkb_device.get_queue_index(vkb::QueueType::compute).value();
        presentQueue = vkb_device.get_queue(vkb::QueueType::present).value();
        presentQueueFamilyIndex = vkb_device.get_queue_index(vkb::QueueType::present).value();

        VmaAllocatorCreateInfo allocator_info{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = physicalDevice,
            .device = device,
            .instance = instance,
        };

        vmaCreateAllocator(&allocator_info, &allocator);

        VkCommandPoolCreateInfo commandPoolInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = graphicsQueueFamilyIndex,
        };

        auto ok = vkCreateCommandPool(device, &commandPoolInfo, nullptr, &singleUseCommandPool);
        assert(ok == VK_SUCCESS);

        std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
        };

        descriptorSetManager[0].OnCreate(this, 10, std::move(descriptorPoolSizes));
        descriptorSetManager[1].OnCreate(this, 10, std::move(descriptorPoolSizes));
    }

    void Device::OnDestroy()
    {
        descriptorSetManager[0].OnDestroy();
        descriptorSetManager[1].OnDestroy();
        vkDestroyCommandPool(device, singleUseCommandPool, nullptr);

        if (surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
        }

        vmaDestroyAllocator(allocator);
        allocator = nullptr;

        if (device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
        }

#ifdef _DEBUG
        vkb::destroy_debug_utils_messenger(instance, debugMessenger, nullptr);
#endif

        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }

    void Device::UploadMesh(const Mesh *mesh)
    {
    }

    VkCommandBuffer Device::GetSingleUseCommandBuffer()
    {
        VkCommandBufferAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = singleUseCommandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1};

        VkCommandBuffer cmd;
        auto ok = vkAllocateCommandBuffers(device, &allocateInfo, &cmd);
        assert(ok == VK_SUCCESS);

        VkCommandBufferBeginInfo beginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };

        ok = vkBeginCommandBuffer(cmd, &beginInfo);
        assert(ok == VK_SUCCESS);

        return cmd;
    }

    void Device::EndSingleUseCommandBuffer(VkCommandBuffer cmd) const
    {
        vkEndCommandBuffer(cmd);

        VkSubmitInfo info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd};

        vkQueueSubmit(graphicsQueue, 1, &info, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, singleUseCommandPool, 1, &cmd);
    }
}
