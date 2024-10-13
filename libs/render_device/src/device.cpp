#include "pch.hpp"

#include "render_device/device.h"
#include "src/vulkan/vulkanDevice.h"

namespace RED
{
std::unique_ptr<Device> Device::Create(void* device, void* queues, void* callbacks)
{
    return std::make_unique<vulkan::VulkanDevice>(device, queues, callbacks);
}

Device::Device()  = default;
Device::~Device() = default;
} // namespace RED
