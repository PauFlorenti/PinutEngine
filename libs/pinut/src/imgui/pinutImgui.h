#pragma once

#include <entt/entt.hpp>
#include <vulkan/vulkan.h>

struct GLFWwindow;
namespace Pinut
{
struct ShaderCameraData;

template <typename ComponentType>
void ComponentRenderDebug(entt::registry& registry, entt::entity entity)
{
    if (registry.any_of<ComponentType>(entity))
    {
        registry.get<ComponentType>(entity).RenderDebug();
    }
}

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
    void Render(entt::registry& registry, const ShaderCameraData& camera);

  private:
    VkDevice         m_device = {VK_NULL_HANDLE};
    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
};
} // namespace Pinut
