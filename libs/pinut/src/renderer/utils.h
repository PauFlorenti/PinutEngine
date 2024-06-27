#pragma once

namespace Pinut
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
} // namespace vkinit
} // namespace Pinut
