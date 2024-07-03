#include "stdafx.h"

#include <array>

#include "forward.h"
#include "src/renderer/common.h"
#include "src/renderer/device.h"
#include "src/renderer/mesh.h"
#include "src/renderer/pipeline.h"
#include "src/renderer/primitives.h"
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

    auto perFrameBinding = vkinit::DescriptorSetLayoutBinding(0,
                                                              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                              1,
                                                              VK_SHADER_STAGE_VERTEX_BIT);
    auto perFrameDescriptorSetLayoutCreateInfo =
      vkinit::DescriptorSetLayoutCreateInfo(1, &perFrameBinding);

    vkCreateDescriptorSetLayout(logicalDevice,
                                &perFrameDescriptorSetLayoutCreateInfo,
                                nullptr,
                                &m_perFrameDescriptorSetLayout);

    auto perObjectBinding = vkinit::DescriptorSetLayoutBinding(0,
                                                               VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                               1,
                                                               VK_SHADER_STAGE_VERTEX_BIT);
    auto perObjectDescriptorSetLayoutCreateInfo =
      vkinit::DescriptorSetLayoutCreateInfo(1, &perObjectBinding);

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
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
    };
    m_descriptorSetManager.OnCreate(logicalDevice, 3, 2, std::move(descriptorPoolSizes));

    m_perFrameBuffer.Create(m_device,
                            sizeof(PerFrameData),
                            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                            VMA_MEMORY_USAGE_AUTO);

    m_perObjectBuffer.Create(m_device,
                             sizeof(glm::mat4),
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                             VMA_MEMORY_USAGE_AUTO);
}

void ForwardPipeline::Shutdown()
{
    const auto device = m_device->GetDevice();

    m_perFrameBuffer.Destroy();
    m_perObjectBuffer.Destroy();

    m_descriptorSetManager.OnDestroy();

    vkDestroyDescriptorSetLayout(device, m_perFrameDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_perObjectDescriptorSetLayout, nullptr);
    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}

void ForwardPipeline::BindPipeline(VkCommandBuffer cmd)
{
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    m_descriptorSetManager.Clear();
}

void ForwardPipeline::UpdatePerFrameData(VkCommandBuffer cmd, PerFrameData data)
{
    auto set = m_descriptorSetManager.Allocate(m_perFrameDescriptorSetLayout);
    vkCmdBindDescriptorSets(cmd,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipelineLayout,
                            0,
                            1,
                            &set,
                            0,
                            nullptr);

    memcpy(m_perFrameBuffer.AllocationInfo().pMappedData, &data, sizeof(PerFrameData));

    auto bufferInfo = vkinit::DescriptorBufferInfo(m_perFrameBuffer.m_buffer, 0, VK_WHOLE_SIZE);
    VkWriteDescriptorSet write =
      vkinit::WriteDescriptorSet(set, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo);
    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &write, 0, nullptr);
}

void ForwardPipeline::Render(VkCommandBuffer cmd)
{
    const auto device = m_device->GetDevice();

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

    auto perFrameBufferInfo =
      vkinit::DescriptorBufferInfo(m_perFrameBuffer.m_buffer, 0, VK_WHOLE_SIZE);
    const auto perFrameWrite = vkinit::WriteDescriptorSet(perObjectDescriptorSet,
                                                          0,
                                                          1,
                                                          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                          &perFrameBufferInfo);
    vkUpdateDescriptorSets(device, 1, &perFrameWrite, 0, nullptr);

    auto perObjectData = (glm::mat4*)m_perObjectBuffer.AllocationInfo().pMappedData;
    *perObjectData     = glm::mat4(1.0f);

    auto perObjectBufferInfo =
      vkinit::DescriptorBufferInfo(m_perObjectBuffer.m_buffer, 0, VK_WHOLE_SIZE);
    auto perObjectWrite = vkinit::WriteDescriptorSet(perObjectDescriptorSet,
                                                     0,
                                                     1,
                                                     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                     &perObjectBufferInfo);
    vkUpdateDescriptorSets(device, 1, &perObjectWrite, 0, nullptr);

    VkDeviceSize offset{0};
    auto         cube = Primitives::GetUnitCube();
    vkCmdBindVertexBuffers(cmd, 0, 1, &cube->m_vertexBuffer.m_buffer, &offset);
    vkCmdBindIndexBuffer(cmd, cube->m_indexBuffer.m_buffer, offset, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, cube->GetIndexCount(), 1, 0, 0, 0);
}

VkPipeline ForwardPipeline::Pipeline() const { return m_pipeline; }
} // namespace Pinut
