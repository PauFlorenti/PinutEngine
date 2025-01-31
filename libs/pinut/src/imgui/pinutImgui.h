#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;
namespace Pinut
{
class PinutImGUI
{
    struct VulkanImguiDeviceInfo
    {
        VkInstance       instance;
        VkDevice         device;
        VkPhysicalDevice physicalDevice;
    };

    struct VulkanImguiQueueInfo
    {
        VkQueue queue{VK_NULL_HANDLE};
        u32     index;
    };

  public:
    explicit PinutImGUI(void*       deviceInfo,
                        void*       queueInfo,
                        GLFWwindow* window,
                        u32         width,
                        u32         height);
    ~PinutImGUI();
    void BeginImGUIRender();

  private:
    VkDevice         m_device = {VK_NULL_HANDLE};
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
};
} // namespace Pinut
