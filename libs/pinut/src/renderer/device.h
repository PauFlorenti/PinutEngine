#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace Pinut
{
class Device final
{
  public:
    Device()  = default;
    ~Device() = default;

    void OnCreate(const std::string& applicationName,
                  const std::string& engineName,
                  bool               bGPUValidationEnabled,
                  GLFWwindow*        window);
    void OnDestroy();

    const VkDevice         GetDevice() const { return m_device; }
    const VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    const VkSurfaceKHR     GetSurface() const { return m_surface; }
    const VmaAllocator     GetAllocator() const { return m_allocator; }

    const VkQueue GetPresentQueue() const { return presentQueue; }
    const VkQueue GetGraphicsQueue() const { return graphicsQueue; }
    const u32     GetGraphicsQueueIndex() const { return graphicsQueueFamilyIndex; }

    VkCommandBuffer CreateImmediateCommandBuffer();
    void            FlushCommandBuffer(VkCommandBuffer cmd) const;

    void WaitIdle() const;

  private:
    VkInstance       m_instance{VK_NULL_HANDLE};
    VkDevice         m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkSurfaceKHR     m_surface{VK_NULL_HANDLE};
    VkCommandPool    m_commandPool{VK_NULL_HANDLE};
    VmaAllocator     m_allocator{nullptr};

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugMessenger{nullptr};
#endif

    VkQueue presentQueue{VK_NULL_HANDLE};
    u32     presentQueueFamilyIndex = 0xFFFFFFFF;
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    u32     graphicsQueueFamilyIndex = 0xFFFFFFFF;
    VkQueue computeQueue{VK_NULL_HANDLE};
    u32     computeQueueFamilyIndex = 0xFFFFFFFF;
};
} // namespace Pinut
