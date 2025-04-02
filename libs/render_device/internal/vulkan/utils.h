#pragma once

#include <common/containers/enumMap.h>

#include "render_device/textureFormat.h"

namespace RED
{
namespace vulkan
{
constexpr common::EnumMap<TextureFormat, VkFormat> FormatToVulkanFormatMap{
  {TextureFormat::UNDEFINED, VK_FORMAT_UNDEFINED},
  {TextureFormat::R8_UNORM, VK_FORMAT_R8_UNORM},
  {TextureFormat::R8_SNORM, VK_FORMAT_R8_SNORM},
  {TextureFormat::R8_UINT, VK_FORMAT_R8_UINT},
  {TextureFormat::R8_SINT, VK_FORMAT_R8_SINT},
  {TextureFormat::R8_SRGB, VK_FORMAT_R8_SRGB},
  {TextureFormat::R16_UINT, VK_FORMAT_R16_UINT},
  {TextureFormat::R16_SINT, VK_FORMAT_R16_SINT},
  {TextureFormat::R16_SFLOAT, VK_FORMAT_R16_SFLOAT},
  {TextureFormat::R32_UINT, VK_FORMAT_R32_UINT},
  {TextureFormat::R32_SINT, VK_FORMAT_R32_SINT},
  {TextureFormat::R32_SFLOAT, VK_FORMAT_R32_SFLOAT},
  {TextureFormat::R8G8_UNORM, VK_FORMAT_R8G8_UNORM},
  {TextureFormat::R8G8_SNORM, VK_FORMAT_R8G8_SNORM},
  {TextureFormat::R8G8_UINT, VK_FORMAT_R8G8_UINT},
  {TextureFormat::R8G8_SINT, VK_FORMAT_R8G8_SINT},
  {TextureFormat::R8G8_SRGB, VK_FORMAT_R8G8_SRGB},
  {TextureFormat::R16G16_UINT, VK_FORMAT_R16G16_UINT},
  {TextureFormat::R16G16_SINT, VK_FORMAT_R16G16_SINT},
  {TextureFormat::R16G16_SFLOAT, VK_FORMAT_R16G16_SFLOAT},
  {TextureFormat::R32G32_UINT, VK_FORMAT_R32G32_UINT},
  {TextureFormat::R32G32_SINT, VK_FORMAT_R32G32_SINT},
  {TextureFormat::R32G32_SFLOAT, VK_FORMAT_R32G32_SFLOAT},
  {TextureFormat::R8G8B8_UNORM, VK_FORMAT_R8G8B8_UNORM},
  {TextureFormat::R8G8B8_SNORM, VK_FORMAT_R8G8B8_SNORM},
  {TextureFormat::R8G8B8_UINT, VK_FORMAT_R8G8B8_UINT},
  {TextureFormat::R8G8B8_SINT, VK_FORMAT_R8G8B8_SINT},
  {TextureFormat::R32G32B32_UINT, VK_FORMAT_R32G32B32_UINT},
  {TextureFormat::R32G32B32_SINT, VK_FORMAT_R32G32B32_SINT},
  {TextureFormat::R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT},
  {TextureFormat::R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM},
  {TextureFormat::R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_SNORM},
  {TextureFormat::R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_UINT},
  {TextureFormat::R8G8B8A8_SINT, VK_FORMAT_R8G8B8A8_SINT},
  {TextureFormat::R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB},
  {TextureFormat::R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_UINT},
  {TextureFormat::R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT},
  {TextureFormat::R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT},
  {TextureFormat::R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_UINT},
  {TextureFormat::R32G32B32A32_SINT, VK_FORMAT_R32G32B32A32_SINT},
  {TextureFormat::R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT},
  {TextureFormat::D16_UNORM, VK_FORMAT_D16_UNORM},
  {TextureFormat::D32_SFLOAT, VK_FORMAT_D32_SFLOAT},
  {TextureFormat::D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT},
  {TextureFormat::D24_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
  {TextureFormat::D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT},
};

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

VkCommandPoolCreateInfo CommandPoolCreateInfo(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags);
} // namespace vkinit
} // namespace vulkan
} // namespace RED
