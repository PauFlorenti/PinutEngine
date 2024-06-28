#include "stdafx.h"

#include "src/renderer/device.h"
#include "forward.h"
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

    auto layout_info = vkinit::PipelineLayoutCreateInfo(0, nullptr);

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
    builder.set_multisampling_none();
    builder.disable_blending();
    builder.enable_depth_test(false, false, VK_COMPARE_OP_NEVER);
    builder.set_depth_format(VK_FORMAT_UNDEFINED);
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
    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}

VkPipeline ForwardPipeline::Pipeline() const { return m_pipeline; }

VkPipelineLayout ForwardPipeline::PipelineLayout() const { return m_pipelineLayout; }
} // namespace Pinut
