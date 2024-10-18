#include "pch.hpp"

#include "render_device/device.h"
#include "src/vulkan/vulkanDevice.h"

namespace RED
{
std::shared_ptr<Device> Device::Create(void* device, void* queues, void* callbacks)
{
    return std::make_shared<vulkan::VulkanDevice>(device, queues, callbacks);
}

Device::Device()  = default;
Device::~Device() = default;
} // namespace RED
