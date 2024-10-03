//#include "stdafx.h"
//
//#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
//#include <external/imgui/backends/imgui_impl_glfw.h>
//#include <external/imgui/backends/imgui_impl_vulkan.h>
//#include <external/imgui/imgui.h>
//// Needs to go after imgui
//#include <ImGuizmo.h>
//
//#include "src/imgui/pinutImgui.h"
//
//namespace Pinut
//{
//void PinutImGUI::Init(Device* device, Swapchain* swapchain, GLFWwindow* window)
//{
//    assert(device);
//    assert(swapchain);
//    assert(window);
//
//    m_device = device->GetDevice();
//
//    VkDescriptorPoolSize descriptor_pool_size = {};
//    descriptor_pool_size.type                 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    descriptor_pool_size.descriptorCount      = 1;
//
//    VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
//    pool_info.poolSizeCount              = 1;
//    pool_info.pPoolSizes                 = &descriptor_pool_size;
//    pool_info.maxSets                    = 1;
//    pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//
//    assert(vkCreateDescriptorPool(device->GetDevice(), &pool_info, nullptr, &m_descriptorPool) ==
//           VK_SUCCESS);
//
//    IMGUI_CHECKVERSION();
//    ImGui::CreateContext();
//
//    const auto format = swapchain->Format();
//
//    ImGui_ImplVulkan_InitInfo imgui_vulkan_info = {
//      .Instance            = device->Instance(),
//      .PhysicalDevice      = device->GetPhysicalDevice(),
//      .Device              = device->GetDevice(),
//      .QueueFamily         = device->GetGraphicsQueueIndex(),
//      .Queue               = device->GetGraphicsQueue(),
//      .DescriptorPool      = m_descriptorPool,
//      .MinImageCount       = 3,
//      .ImageCount          = 3,
//      .UseDynamicRendering = true,
//      .PipelineRenderingCreateInfo =
//        {
//          .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
//          .colorAttachmentCount    = 1,
//          .pColorAttachmentFormats = &format,
//        },
//    };
//
//    ImGuiIO& io = ImGui::GetIO();
//    io.DisplaySize =
//      ImVec2(static_cast<f32>(swapchain->Width()), static_cast<f32>(swapchain->Height()));
//    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
//
//    if (!ImGui_ImplVulkan_Init(&imgui_vulkan_info) || !ImGui_ImplGlfw_InitForVulkan(window, true))
//    {
//        printf("Failed to initialize imgui for vulkan backend.");
//    }
//
//    ImGui_ImplVulkan_CreateFontsTexture();
//}
//
//void PinutImGUI::Shutdown()
//{
//    ImGui_ImplVulkan_DestroyFontsTexture();
//    ImGui_ImplVulkan_Shutdown();
//    ImGui_ImplGlfw_Shutdown();
//    ImGui::DestroyContext();
//    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
//}
//
//void PinutImGUI::BeginImGUIRender(VkCommandBuffer cmd)
//{
//    ImGui_ImplVulkan_NewFrame();
//    ImGui_ImplGlfw_NewFrame();
//    ImGui::NewFrame();
//    ImGuizmo::BeginFrame();
//}
//
//void PinutImGUI::EndImGUIRender(VkCommandBuffer cmd, u32 width, u32 height, VkImageView imageView)
//{
//    ImGui::Render();
//
//    auto attachment = vkinit::RenderingAttachmentInfo(imageView,
//                                                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
//                                                      VK_ATTACHMENT_LOAD_OP_LOAD,
//                                                      VK_ATTACHMENT_STORE_OP_STORE,
//                                                      {0.0f, 0.f, 0.f, 0.f});
//
//    const auto renderingInfo =
//      vkinit::RenderingInfo(1, &attachment, {0, 0, std::move(width), std::move(height)});
//
//    vkCmdBeginRendering(cmd, &renderingInfo);
//    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
//    vkCmdEndRendering(cmd);
//}
//} // namespace Pinut
