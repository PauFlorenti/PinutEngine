#include "stdafx.h"

#include <array>

#include "forward.h"
#include "src/assets/mesh.h"
#include "src/assets/texture.h"
#include "src/core/camera.h"
#include "src/core/scene.h"
#include "src/renderer/common.h"
#include "src/renderer/device.h"
#include "src/renderer/pipeline.h"
#include "src/renderer/renderable.h"
#include "src/renderer/utils.h"

namespace Pinut
{
void ForwardPipeline::Init(Device* device)
{
    assert(device);
    m_device = device;

    auto logicalDevice = m_device->GetDevice();

    VkShaderModule vertex_shader;
    if (!vkinit::load_shader_module("shaders/basic.vert.spv", logicalDevice, &vertex_shader))
    {
        printf("[ERROR]: Error building the forward vertex shader.");
    }

    VkShaderModule fragment_shader;
    if (!vkinit::load_shader_module("shaders/basic.frag.spv", logicalDevice, &fragment_shader))
    {
        printf("[ERROR]: Error building the forward fragment shader.");
    }

    // clang-format off
    std::vector<VkVertexInputAttributeDescription> input_attributes{
        vkinit::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)),
        vkinit::VertexInputAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)),
        vkinit::VertexInputAttributeDescription(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)),
        vkinit::VertexInputAttributeDescription(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)),
    };
    // clang-format on

    VkDescriptorSetLayoutBinding perFrameBindings[2] = {
      vkinit::DescriptorSetLayoutBinding(0,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         1,
                                         VK_SHADER_STAGE_VERTEX_BIT),
      vkinit::DescriptorSetLayoutBinding(1,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         1,
                                         VK_SHADER_STAGE_FRAGMENT_BIT),
    };
    auto perFrameDescriptorSetLayoutCreateInfo =
      vkinit::DescriptorSetLayoutCreateInfo(2, perFrameBindings);

    vkCreateDescriptorSetLayout(logicalDevice,
                                &perFrameDescriptorSetLayoutCreateInfo,
                                nullptr,
                                &m_perFrameDescriptorSetLayout);

    VkDescriptorSetLayoutBinding perObjectBindings[2] = {
      vkinit::DescriptorSetLayoutBinding(0,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         1,
                                         VK_SHADER_STAGE_VERTEX_BIT),
      vkinit::DescriptorSetLayoutBinding(1,
                                         VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                         1,
                                         VK_SHADER_STAGE_VERTEX_BIT),
    };

    auto perObjectDescriptorSetLayoutCreateInfo =
      vkinit::DescriptorSetLayoutCreateInfo(2, perObjectBindings);

    vkCreateDescriptorSetLayout(logicalDevice,
                                &perObjectDescriptorSetLayoutCreateInfo,
                                nullptr,
                                &m_perObjectDescriptorSetLayout);

    std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {m_perFrameDescriptorSetLayout,
                                                                 m_perObjectDescriptorSetLayout};
    auto layout_info = vkinit::PipelineLayoutCreateInfo(2, descriptorSetLayouts.data());

    auto ok = vkCreatePipelineLayout(logicalDevice, &layout_info, nullptr, &m_pipelineLayout);
    assert(ok == VK_SUCCESS);

    PipelineBuilder builder;
    builder.layout = m_pipelineLayout;
    builder.set_shaders(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
    builder.set_shaders(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.set_rasterizer(VK_POLYGON_MODE_FILL,
                           VK_CULL_MODE_NONE,
                           VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.set_input_attribute(std::move(input_attributes), sizeof(Vertex));
    builder.set_multisampling_none();
    builder.disable_blending();
    builder.enable_depth_test(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
    builder.set_depth_format(VK_FORMAT_D32_SFLOAT);
    builder.set_stencil_format(VK_FORMAT_UNDEFINED);
    builder.set_color_attachment_format(VK_FORMAT_B8G8R8A8_UNORM);

    m_pipeline = builder.build(logicalDevice);

    vkDestroyShaderModule(logicalDevice, vertex_shader, nullptr);
    vkDestroyShaderModule(logicalDevice, fragment_shader, nullptr);

    std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4},
    };
    m_descriptorSetManager.OnCreate(logicalDevice, 3, 3, std::move(descriptorPoolSizes));

    m_perFrameBuffer.Create(m_device,
                            sizeof(PerFrameData),
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VMA_MEMORY_USAGE_AUTO);

    m_perObjectBuffer.Create(m_device,
                             sizeof(u32),
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VMA_MEMORY_USAGE_AUTO);

    m_transformsBuffer.Create(m_device,
                              sizeof(glm::mat4) * MAX_ENTITIES,
                              VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                              VMA_MEMORY_USAGE_AUTO);

    m_lightsBuffer.Create(m_device,
                          sizeof(Light) * MAX_LIGHTS,
                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VMA_MEMORY_USAGE_AUTO);
}

void ForwardPipeline::Shutdown()
{
    const auto device = m_device->GetDevice();

    OnDestroyWindowDependantResources();

    m_lightsBuffer.Destroy();
    m_transformsBuffer.Destroy();
    m_perObjectBuffer.Destroy();
    m_perFrameBuffer.Destroy();

    m_descriptorSetManager.OnDestroy();

    vkDestroyDescriptorSetLayout(device, m_perFrameDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_perObjectDescriptorSetLayout, nullptr);
    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}

void ForwardPipeline::OnCreateWindowDependantResources(u32 width, u32 height)
{
    VkImageCreateInfo depthTextureInfo{
      .sType                 = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .pNext                 = nullptr,
      .imageType             = VK_IMAGE_TYPE_2D,
      .format                = VK_FORMAT_D32_SFLOAT,
      .extent                = {width, height, 1},
      .mipLevels             = 1,
      .arrayLayers           = 1,
      .samples               = VK_SAMPLE_COUNT_1_BIT,
      .tiling                = VK_IMAGE_TILING_OPTIMAL,
      .usage                 = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
      .sharingMode           = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices   = nullptr,
      .initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    m_depthTexture = new Texture();
    m_depthTexture->Create(m_device, depthTextureInfo);
}

void ForwardPipeline::OnDestroyWindowDependantResources()
{
    m_device->WaitIdle();

    if (m_depthTexture != nullptr)
    {
        m_depthTexture->Destroy();
        m_depthTexture = nullptr;
    }
}

void ForwardPipeline::BindPipeline(VkCommandBuffer cmd)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    m_descriptorSetManager.Clear();
}

void ForwardPipeline::Render(VkCommandBuffer cmd, Camera* camera, Scene* scene)
{
    const auto device = m_device->GetDevice();

    // Camera
    {
        auto data = (PerFrameData*)m_perFrameBuffer.AllocationInfo().pMappedData;

        data->view           = camera->View();
        data->projection     = camera->Projection();
        data->cameraPosition = camera->Position();
    }

    // Lights
    {
        const auto lightsCount = static_cast<u32>(scene->Lights().size());
        auto       lightData   = (LightData*)m_lightsBuffer.AllocationInfo().pMappedData;
        lightData->lightsCount = lightsCount;
        memcpy(lightData->lights, scene->Lights().data(), sizeof(Light) * lightsCount);
    }

    auto set = m_descriptorSetManager.Allocate(m_perFrameDescriptorSetLayout);
    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipelineLayout,
                            0,
                            1,
                            &set,
                            0,
                            nullptr);

    auto bufferVertexInfo =
      vkinit::DescriptorBufferInfo(m_perFrameBuffer.m_buffer, 0, sizeof(PerFrameData));
    auto lightsBufferInfo = vkinit::DescriptorBufferInfo(m_lightsBuffer.m_buffer, 0, VK_WHOLE_SIZE);
    VkWriteDescriptorSet perFrameWrites[2] = {
      vkinit::WriteDescriptorSet(set, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferVertexInfo),
      vkinit::WriteDescriptorSet(set, 1, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &lightsBufferInfo),
    };
    vkUpdateDescriptorSets(m_device->GetDevice(), 2, perFrameWrites, 0, nullptr);

    if (!scene)
        return;

    auto& renderables = scene->Renderables();
    for (u32 i = 0; i < renderables.size(); i++)
    {
        auto& renderable = renderables[i];
        auto  transforms = (glm::mat4*)m_transformsBuffer.AllocationInfo().pMappedData;
        transforms[i]    = renderable->Model();
        renderable->SetInstanceIndex(i);
    }

    for (auto& r : scene->Renderables())
    {
        // TODO if material is same, change it.
        const auto perObjectDescriptorSet =
          m_descriptorSetManager.Allocate(m_perObjectDescriptorSetLayout);
        vkCmdBindDescriptorSets(cmd,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipelineLayout,
                                1,
                                1,
                                &perObjectDescriptorSet,
                                0,
                                nullptr);

        auto perObjectData = (u32*)m_perObjectBuffer.AllocationInfo().pMappedData;
        *perObjectData     = 0xFFFFFFFF;

        auto perObjectBufferInfo =
          vkinit::DescriptorBufferInfo(m_perObjectBuffer.m_buffer, 0, sizeof(u32));

        auto transformBufferInfo = vkinit::DescriptorBufferInfo(m_transformsBuffer.m_buffer,
                                                                0,
                                                                sizeof(glm::mat4) * MAX_ENTITIES);

        VkWriteDescriptorSet writes[2] = {
          vkinit::WriteDescriptorSet(perObjectDescriptorSet,
                                     0,
                                     1,
                                     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                     &perObjectBufferInfo),
          vkinit::WriteDescriptorSet(perObjectDescriptorSet,
                                     1,
                                     1,
                                     VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                     &transformBufferInfo),
        };

        vkUpdateDescriptorSets(device, 2, writes, 0, nullptr);

        r->Draw(cmd);
    }
}

VkPipeline ForwardPipeline::Pipeline() const { return m_pipeline; }
} // namespace Pinut
