#pragma once

namespace RED
{
namespace vulkan
{
namespace vkinit
{
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

VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(u32 numberSets,
                                                    const std::vector<VkDescriptorPoolSize>& sizes);

VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool,
                                                      u32              descriptorSetCount,
                                                      const VkDescriptorSetLayout& layout);

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

VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool        commandPool,
                                                      VkCommandBufferLevel level,
                                                      u32                  cmdCount);

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

VkCommandPoolCreateInfo CommandPoolCreateInfo(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags);
} // namespace vkinit
} // namespace vulkan
} // namespace RED
