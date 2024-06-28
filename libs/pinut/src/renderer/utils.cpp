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
} // namespace vkinit
} // namespace Pinut
