//#pragma once
//
//#include <vulkan/vulkan.h>
//
//struct GLFWwindow;
//namespace Pinut
//{
//class Device;
//class Swapchain;
//class PinutImGUI
//{
//  public:
//    void Init(Device* device, Swapchain* swapchain, GLFWwindow* window);
//    void Shutdown();
//    void BeginImGUIRender(VkCommandBuffer cmd);
//    void EndImGUIRender(VkCommandBuffer cmd, u32 width, u32 height, VkImageView imageView);
//
//  private:
//    VkDevice         m_device = {VK_NULL_HANDLE};
//    VkDescriptorPool m_descriptorPool{VK_NULL_HANDLE};
//};
//} // namespace Pinut
