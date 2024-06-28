#pragma once

namespace Pinut
{
namespace vkinit
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
    void       set_rasterizer(VkPolygonMode   polygon_mode,
                              VkCullModeFlags cull_mode_flags,
                              VkFrontFace     front_face);
    void       set_multisampling_none();
    void       set_input_attribute(std::vector<VkVertexInputAttributeDescription> attr_descriptions,
                                   size_t                                         stride_size);
    void       disable_blending();
    void       set_color_attachment_format(VkFormat format);
    void       set_depth_format(VkFormat format);
    void       set_stencil_format(VkFormat format);
    void       enable_depth_test(bool        depth_test_enable,
                                 bool        write_enable,
                                 VkCompareOp compare_operation);
    void       disable_depth_test();
    VkPipeline build(VkDevice device);
};

VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t           binding,
                                                        VkDescriptorType   descriptorType,
                                                        uint32_t           descriptorCount,
                                                        VkShaderStageFlags stageFlags);

VkVertexInputAttributeDescription VertexInputAttributeDescription(uint32_t location,
                                                                  uint32_t binding,
                                                                  VkFormat format,
                                                                  uint32_t offset);

VkDescriptorSetLayoutCreateInfo
DescriptorSetLayoutCreateInfo(uint32_t bindingCount, VkDescriptorSetLayoutBinding* bindings);

VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(uint32_t               layoutCount,
                                                    VkDescriptorSetLayout* layouts);

VkDescriptorBufferInfo DescriptorBufferInfo(VkBuffer     buffer,
                                            VkDeviceSize offset,
                                            VkDeviceSize range);

VkDescriptorImageInfo DescriptorImageInfo(VkImageView   imageView,
                                          VkSampler     sampler,
                                          VkImageLayout layout);

VkWriteDescriptorSet WriteDescriptorSet(VkDescriptorSet         set,
                                        uint32_t                binding,
                                        uint32_t                descriptorCount,
                                        VkDescriptorType        type,
                                        VkDescriptorBufferInfo* bufferInfo = nullptr,
                                        VkDescriptorImageInfo*  imageInfo  = nullptr);

VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

VkRenderingAttachmentInfo RenderingAttachmentInfo(VkImageView         imageView,
                                                  VkImageLayout       layout,
                                                  VkAttachmentLoadOp  loadOp,
                                                  VkAttachmentStoreOp storeOp,
                                                  VkClearValue        clearValue);

VkRenderingInfo RenderingInfo(uint32_t                   attachmentCount,
                              VkRenderingAttachmentInfo* attachments,
                              VkRect2D                   extent,
                              VkRenderingAttachmentInfo* depthAttachment = VK_NULL_HANDLE);

bool load_shader_module(const char* filename, VkDevice device, VkShaderModule* out_shader_module);
} // namespace vkinit
} // namespace Pinut
