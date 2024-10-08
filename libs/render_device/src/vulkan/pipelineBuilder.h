#pragma once

#include <vulkan/vulkan.h>

struct BlendState;
struct RasterState;
namespace vulkan
{
struct PipelineBuilder
{
    std::vector<VkPipelineShaderStageCreateInfo>   shader_stages;
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions;

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    VkVertexInputBindingDescription        input_binding{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkPipelineColorBlendAttachmentState    color_blend_attachment{};
    VkPipelineMultisampleStateCreateInfo   multisampling{};
    VkPipelineLayout                       layout{};
    VkPipelineDepthStencilStateCreateInfo  depth_stencil{};
    VkPipelineRenderingCreateInfo          render_info{};
    VkFormat                               color_attachment_format{};

    PipelineBuilder() { clear(); }
    void       clear();
    void       set_shaders(VkShaderModule shader_module, VkShaderStageFlags shader_stage);
    void       set_topology(VkPrimitiveTopology topology);
    void       set_rasterizer(const RasterState& state);
    void       set_multisampling_none();
    void       set_input_attribute(std::vector<VkVertexInputAttributeDescription> attr_descriptions,
                                   size_t                                         stride_size);
    void       disable_blending();
    void       enable_blending();
    void       setBlendingState(const BlendState& state);
    void       set_color_attachment_format(VkFormat format);
    void       set_depth_format(VkFormat format);
    void       set_stencil_format(VkFormat format);
    void       enable_depth_test(bool        depth_test_enable,
                                 bool        write_enable,
                                 VkCompareOp compare_operation = VK_COMPARE_OP_NEVER);
    void       disable_depth_test();
    VkPipeline build(VkDevice device);
};
} // namespace vulkan
