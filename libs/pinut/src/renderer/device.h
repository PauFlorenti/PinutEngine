#pragma once

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

  private:
    VkInstance       instance{VK_NULL_HANDLE};
    VkDevice         device{VK_NULL_HANDLE};
    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    VkSurfaceKHR     surface{VK_NULL_HANDLE};

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
