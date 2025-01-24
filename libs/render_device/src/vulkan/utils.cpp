#include "pch.hpp"

#include <fstream>

#include "src/vulkan/utils.h"

namespace RED
{
namespace vulkan
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

VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(u32 numberSets,
                                                    const std::vector<VkDescriptorPoolSize>& sizes)
{
    VkDescriptorPoolCreateInfo info{
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext         = nullptr,
      .maxSets       = numberSets,
      .poolSizeCount = static_cast<u32>(sizes.size()),
      .pPoolSizes    = sizes.data(),
    };

    return info;
}

VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool,
                                                      u32              descriptorSetCount,
                                                      const VkDescriptorSetLayout& layout)
{
    VkDescriptorSetAllocateInfo info{.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                                     .descriptorPool     = descriptorPool,
                                     .descriptorSetCount = descriptorSetCount,
                                     .pSetLayouts        = &layout};

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

VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool        commandPool,
                                                      VkCommandBufferLevel level,
                                                      u32                  cmdCount)
{
    VkCommandBufferAllocateInfo info{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext              = nullptr,
      .commandPool        = commandPool,
      .level              = level,
      .commandBufferCount = cmdCount,
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
      .pStencilAttachment   = nullptr,
    };

    return info;
}

VkCommandPoolCreateInfo CommandPoolCreateInfo(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo info{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .pNext            = nullptr,
      .flags            = flags,
      .queueFamilyIndex = queueFamilyIndex,
    };

    return info;
}
} // namespace vkinit
} // namespace vulkan
} // namespace RED
