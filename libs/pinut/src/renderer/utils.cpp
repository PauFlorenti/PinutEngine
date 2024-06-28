#include "stdafx.h"

#include "utils.h"
#include <fstream>

namespace Pinut
{
namespace vkinit
{
VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t           binding,
                                                        VkDescriptorType   descriptorType,
                                                        uint32_t           descriptorCount,
                                                        VkShaderStageFlags stageFlags)
{
    VkDescriptorSetLayoutBinding layoutBinding{
      .binding            = binding,
      .descriptorType     = descriptorType,
      .descriptorCount    = descriptorCount,
      .stageFlags         = stageFlags,
      .pImmutableSamplers = nullptr,
    };

    return layoutBinding;
}

VkVertexInputAttributeDescription VertexInputAttributeDescription(uint32_t location,
                                                                  uint32_t binding,
                                                                  VkFormat format,
                                                                  uint32_t offset)
{
    VkVertexInputAttributeDescription info{
      .location = location,
      .binding  = binding,
      .format   = format,
      .offset   = offset,
    };

    return info;
}

VkDescriptorSetLayoutCreateInfo
DescriptorSetLayoutCreateInfo(uint32_t bindingCount, VkDescriptorSetLayoutBinding* bindings)
{
    VkDescriptorSetLayoutCreateInfo info{
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = bindingCount,
      .pBindings    = bindings,
    };

    return info;
}

VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(uint32_t               layoutCount,
                                                    VkDescriptorSetLayout* layouts)
{
    VkPipelineLayoutCreateInfo info = {
      .sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = layoutCount,
      .pSetLayouts    = layouts,
    };

    return info;
}

VkDescriptorBufferInfo DescriptorBufferInfo(VkBuffer     buffer,
                                            VkDeviceSize offset,
                                            VkDeviceSize range)
{
    VkDescriptorBufferInfo info{
      .buffer = buffer,
      .offset = offset,
      .range  = range,
    };

    return info;
}

VkDescriptorImageInfo DescriptorImageInfo(VkImageView   imageView,
                                          VkSampler     sampler,
                                          VkImageLayout layout)
{
    VkDescriptorImageInfo info{
      .sampler     = sampler,
      .imageView   = imageView,
      .imageLayout = layout,
    };

    return info;
}

VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet         set,
                                        uint32_t                binding,
                                        uint32_t                descriptorCount,
                                        VkDescriptorType        type,
                                        VkDescriptorBufferInfo* bufferInfo,
                                        VkDescriptorImageInfo*  imageInfo)
{
    VkWriteDescriptorSet write{
      .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
      .dstSet          = set,
      .dstBinding      = binding,
      .descriptorCount = descriptorCount,
      .descriptorType  = type,
      .pImageInfo      = imageInfo,
      .pBufferInfo     = bufferInfo,
    };

    return write;
}

VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags)
{
    VkCommandBufferBeginInfo info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = flags,
    };

    return info;
}

VkRenderingAttachmentInfo RenderingAttachmentInfo(VkImageView         imageView,
                                                  VkImageLayout       layout,
                                                  VkAttachmentLoadOp  loadOp,
                                                  VkAttachmentStoreOp storeOp,
                                                  VkClearValue        clearValue)
{
    VkRenderingAttachmentInfo info{
      .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
      .pNext       = nullptr,
      .imageView   = imageView,
      .imageLayout = layout,
      .loadOp      = loadOp,
      .storeOp     = storeOp,
      .clearValue  = clearValue,
    };

    return info;
}

VkRenderingInfo RenderingInfo(uint32_t                   attachmentCount,
                              VkRenderingAttachmentInfo* attachments,
                              VkRect2D                   extent,
                              VkRenderingAttachmentInfo* depthAttachment)
{
    VkRenderingInfo info{
      .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
      .pNext                = nullptr,
      .renderArea           = extent,
      .layerCount           = 1,
      .colorAttachmentCount = attachmentCount,
      .pColorAttachments    = attachments,
      .pDepthAttachment     = depthAttachment,
      .pStencilAttachment   = depthAttachment,
    };

    return info;
}

bool load_shader_module(const char* filename, VkDevice device, VkShaderModule* out_shader_module)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        return false;

    const size_t          file_size = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

    file.seekg(0);
    file.read((char*)buffer.data(), file_size);
    file.close();

    VkShaderModuleCreateInfo info{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = buffer.size() * sizeof(uint32_t),
      .pCode    = buffer.data(),
    };

    VkShaderModule module;
    if (vkCreateShaderModule(device, &info, nullptr, &module) != VK_SUCCESS)
        return false;

    *out_shader_module = module;
    return true;
}

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
} // namespace vkinit
} // namespace Pinut
