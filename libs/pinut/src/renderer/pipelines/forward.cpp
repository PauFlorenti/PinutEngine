#include "stdafx.h"

#include <array>

#include "forward.h"
#include "src/renderer/device.h"
#include "src/renderer/mesh.h"
#include "src/renderer/pipeline.h"
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
}

void ForwardPipeline::Render() {}

void ForwardPipeline::Shutdown()
{
    auto device = m_device->GetDevice();
    vkDestroyDescriptorSetLayout(device, m_perFrameDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_perObjectDescriptorSetLayout, nullptr);
    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}

VkPipeline ForwardPipeline::Pipeline() const { return m_pipeline; }

VkPipelineLayout ForwardPipeline::PipelineLayout() const { return m_pipelineLayout; }
} // namespace Pinut
