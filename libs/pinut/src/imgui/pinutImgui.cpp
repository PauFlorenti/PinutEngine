#include "pch.hpp"

#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/imgui.h>
// Needs to go after imgui
#include <ImGuizmo.h>

#include "render_device/device.h"

#include "pinut/components/lightComponent.h"
#include "pinut/components/meshComponent.h"
#include "pinut/components/renderComponent.h"
#include "pinut/components/transformComponent.h"
#include "pinut/imgui/pinutImgui.h"
#include "pinut/renderer/common.h"

namespace Pinut
{
PinutImGUI::PinutImGUI(void* deviceInfo, void* queueInfo, GLFWwindow* window, u32 width, u32 height)
{
    assert(deviceInfo);
    assert(queueInfo);
    assert(window);

    auto vulkanDeviceInfo = reinterpret_cast<VulkanImguiDeviceInfo*>(deviceInfo);
    auto queue            = reinterpret_cast<VulkanImguiQueueInfo*>(queueInfo);

    m_device = vulkanDeviceInfo->device;

    VkDescriptorPoolSize descriptor_pool_size = {};
    descriptor_pool_size.type                 = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_pool_size.descriptorCount      = 1;

    VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.poolSizeCount              = 1;
    pool_info.pPoolSizes                 = &descriptor_pool_size;
    pool_info.maxSets                    = 1;
    pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    assert(
      vkCreateDescriptorPool(vulkanDeviceInfo->device, &pool_info, nullptr, &m_descriptorPool) ==
      VK_SUCCESS);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    const auto format = VK_FORMAT_R32G32B32A32_SFLOAT;

    ImGui_ImplVulkan_InitInfo imgui_vulkan_info = {
      .Instance            = vulkanDeviceInfo->instance,
      .PhysicalDevice      = vulkanDeviceInfo->physicalDevice,
      .Device              = vulkanDeviceInfo->device,
      .QueueFamily         = queue->index,
      .Queue               = queue->queue,
      .DescriptorPool      = m_descriptorPool,
      .MinImageCount       = 3,
      .ImageCount          = 3,
      .UseDynamicRendering = true,
      .PipelineRenderingCreateInfo =
        {
          .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
          .colorAttachmentCount    = 1,
          .pColorAttachmentFormats = &format,
        },
    };

    ImGuiIO& io                = ImGui::GetIO();
    io.DisplaySize             = ImVec2(static_cast<f32>(width), static_cast<f32>(height));
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    if (!ImGui_ImplVulkan_Init(&imgui_vulkan_info) || !ImGui_ImplGlfw_InitForVulkan(window, true))
    {
        printf("Failed to initialize imgui for vulkan backend.");
    }

    ImGui_ImplVulkan_CreateFontsTexture();
}

PinutImGUI::~PinutImGUI()
{
    ImGui_ImplVulkan_DestroyFontsTexture();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
}

void PinutImGUI::BeginImGUIRender()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}
} // namespace Pinut

void Pinut::PinutImGUI::Render(entt::registry& registry, const ShaderCameraData& camera)
{
    ImGui::Begin("Entities");

    ImGui::Text("FPS: %f", &ImGui::GetIO().Framerate);

    registry.view<entt::entity>().each(
      [&](auto entity)
      {
          std::string label = "Entity " + std::to_string(static_cast<int>(entity));
          if (ImGui::TreeNode(label.c_str()))
          {
              registry.get<Component::TransformComponent>(entity).RenderDebug(camera);
              ComponentRenderDebug<Component::LightComponent>(registry, entity);
              ComponentRenderDebug<Component::MeshComponent>(registry, entity);
              ComponentRenderDebug<Component::RenderComponent>(registry, entity);

              ImGui::TreePop();
          }
      });

    ImGui::End();
}
