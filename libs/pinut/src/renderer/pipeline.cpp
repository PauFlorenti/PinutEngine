#include "stdafx.h"

#include "pipeline.h"

namespace Pinut
{
void PipelineBuilder::clear()
{
    shader_stages.clear();
    input_assembly         = {.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    rasterizer             = {.sType     = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                              .lineWidth = 1.0f};
    color_blend_attachment = {};
    multisampling          = {.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    layout                 = {};
    depth_stencil          = {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    render_info            = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
}

void PipelineBuilder::set_shaders(VkShaderModule shader_module, VkShaderStageFlags shader_stage)
{
    VkPipelineShaderStageCreateInfo info{
      .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage  = (VkShaderStageFlagBits)shader_stage,
      .module = shader_module,
      .pName  = "main",
    };

    shader_stages.push_back(std::move(info));
}

void PipelineBuilder::set_topology(VkPrimitiveTopology topology)
{
    input_assembly.topology               = topology;
    input_assembly.primitiveRestartEnable = VK_FALSE;
}

void PipelineBuilder::set_rasterizer(VkPolygonMode   polygon_mode,
                                     VkCullModeFlags cull_mode_flags,
                                     VkFrontFace     front_face)
{
    rasterizer.polygonMode = polygon_mode;
    rasterizer.frontFace   = front_face;
    rasterizer.cullMode    = cull_mode_flags;
}

void PipelineBuilder::set_multisampling_none()
{
    multisampling.sampleShadingEnable   = VK_FALSE;
    multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading      = 0.0f;
    multisampling.pSampleMask           = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable      = VK_FALSE;
}

void PipelineBuilder::set_input_attribute(
  std::vector<VkVertexInputAttributeDescription> attr_descriptions,
  size_t                                         stride_size)
{
    attribute_descriptions = std::move(attr_descriptions);

    if (stride_size != 0)
    {
        input_binding.binding   = 0;
        input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        input_binding.stride    = static_cast<uint32_t>(stride_size);
    }
}

void PipelineBuilder::disable_blending()
{
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
}

void PipelineBuilder::set_color_attachment_format(VkFormat format)
{
    color_attachment_format             = format;
    render_info.colorAttachmentCount    = 1;
    render_info.pColorAttachmentFormats = &color_attachment_format;
}

void PipelineBuilder::set_depth_format(VkFormat format)
{
    render_info.depthAttachmentFormat = format;
}

void PipelineBuilder::set_stencil_format(VkFormat format)
{
    render_info.stencilAttachmentFormat = format;
}

void PipelineBuilder::enable_depth_test(bool        depth_test_enable,
                                        bool        write_enable,
                                        VkCompareOp compare_operation)
{
    depth_stencil                  = {};
    depth_stencil.depthTestEnable  = depth_test_enable ? VK_TRUE : VK_FALSE;
    depth_stencil.depthWriteEnable = write_enable ? VK_TRUE : VK_FALSE;
    depth_stencil.depthCompareOp   = depth_test_enable ? compare_operation : VK_COMPARE_OP_NEVER;
    depth_stencil.back.compareOp   = VK_COMPARE_OP_ALWAYS;
}

void PipelineBuilder::disable_depth_test()
{
    depth_stencil.depthTestEnable       = VK_FALSE;
    depth_stencil.depthWriteEnable      = VK_FALSE;
    depth_stencil.depthCompareOp        = VK_COMPARE_OP_NEVER;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable     = VK_FALSE;
    depth_stencil.front                 = {};
    depth_stencil.back                  = {};
    depth_stencil.minDepthBounds        = 0.0f;
    depth_stencil.maxDepthBounds        = 1.0f;
}

VkPipeline PipelineBuilder::build(VkDevice device)
{
    VkPipelineVertexInputStateCreateInfo vertex_input_info{
      .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount   = static_cast<uint32_t>(input_binding.stride == 0 ? 0 : 1),
      .pVertexBindingDescriptions      = input_binding.stride == 0 ? nullptr : &input_binding,
      .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
      .pVertexAttributeDescriptions    = attribute_descriptions.data(),
    };

    VkViewport viewport{
      .x = 0,
      .y = 0,
    };

    VkRect2D scissors{
      .offset = {0, 0},
      .extent = {0, 0},
    };

    VkPipelineViewportStateCreateInfo viewport_state{
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .viewportCount = 1,
      .scissorCount  = 1,
    };

    VkPipelineColorBlendStateCreateInfo color_blend{
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .logicOpEnable   = VK_TRUE,
      .logicOp         = VK_LOGIC_OP_COPY,
      .attachmentCount = 1,
      .pAttachments    = &color_blend_attachment,
    };

    VkDynamicState state[] = {
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamic_state{
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .dynamicStateCount = 2,
      .pDynamicStates    = state,
    };

    VkGraphicsPipelineCreateInfo info{
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext               = &render_info,
      .stageCount          = static_cast<uint32_t>(shader_stages.size()),
      .pStages             = shader_stages.data(),
      .pVertexInputState   = &vertex_input_info,
      .pInputAssemblyState = &input_assembly,
      .pViewportState      = &viewport_state,
      .pRasterizationState = &rasterizer,
      .pMultisampleState   = &multisampling,
      .pDepthStencilState  = &depth_stencil,
      .pColorBlendState    = &color_blend,
      .pDynamicState       = &dynamic_state,
      .layout              = layout,
    };

    VkPipeline pipeline{VK_NULL_HANDLE};
    auto       ok = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);
    assert(ok == VK_SUCCESS);
    return pipeline;
}
} // namespace Pinut
