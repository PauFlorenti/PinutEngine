#include "pch.hpp"

#include "render_device/renderPipeline.h"
#include "render_device/states.h"

#include "vulkan/utils.h"
#include "vulkan/vulkanPipeline.h"
#include "vulkan/vulkanPipelineBuilder.h"
#include "vulkan/vulkanTexture.h"
#include "vulkan/vulkanVertexDeclaration.h"

namespace RED
{
namespace vulkan
{
// TODO Temporal?
std::vector<VkDescriptorPoolSize> globalPoolSizes = {
  {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
  {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10},
};

VulkanPipeline::VulkanPipeline()                      = default;
VulkanPipeline::VulkanPipeline(const VulkanPipeline&) = default;
VulkanPipeline::~VulkanPipeline()                     = default;

VulkanPipeline VulkanPipeline::Create(VkDevice              device,
                                      const RenderPipeline& renderPipeline,
                                      const GraphicsState&  graphicsState)
{
    VulkanPipeline pipeline;

    VkShaderModule vertexShaderModule, fragmentShaderModule;
    const auto     vertexShaderPath   = "shaders/" + renderPipeline.vertexShader.name + ".spv";
    const auto     fragmentShaderPath = "shaders/" + renderPipeline.fragmentShader.name + ".spv";
    vertexShaderModule                = CreateShaderModule(device, vertexShaderPath.c_str());
    fragmentShaderModule              = renderPipeline.fragmentShader.name.empty() ?
                                          VK_NULL_HANDLE :
                                          CreateShaderModule(device, fragmentShaderPath.c_str());

    if (!vertexShaderModule)
    {
        printf("[ERROR]: Failed to create vertex shader.");
        return {};
    }

    if (!fragmentShaderModule)
    {
        printf("[DEBUG]: Fragment shader is null. Assuming only depth/stencil pipeline.");
    }

    pipeline.m_descriptorSetLayouts = CreateDescriptorSetLayouts(device, renderPipeline);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.reserve(pipeline.m_descriptorSetLayouts.size());

    for (const auto& result : pipeline.m_descriptorSetLayouts)
    {
        if (result.descriptorSetLayout != VK_NULL_HANDLE)
            descriptorSetLayouts.emplace_back(result.descriptorSetLayout);
    }

    auto pipelineLayoutInfo =
      vkinit::PipelineLayoutCreateInfo(static_cast<u32>(descriptorSetLayouts.size()),
                                       descriptorSetLayouts.data());
    assert(
      vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline.m_pipelineLayout) ==
      VK_SUCCESS);

    const auto vertexDeclaration =
      getVertexDeclarationByName(renderPipeline.inputVertexDeclaration);
    const auto depthFormat = FormatToVulkanFormatMap[graphicsState.depth.depthFormat];

    std::vector<VkFormat> attachmentFormats(renderPipeline.attachmentFormats.size());
    std::transform(renderPipeline.attachmentFormats.begin(),
                   renderPipeline.attachmentFormats.end(),
                   attachmentFormats.begin(),
                   [&](const TextureFormat& InFormat)
                   {
                       return FormatToVulkanFormatMap[InFormat];
                   });

    PipelineBuilder builder;
    builder.layout = pipeline.m_pipelineLayout;
    builder.set_shaders(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    builder.set_shaders(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.set_topology(graphicsState.topology);
    builder.set_rasterizer(graphicsState.raster);
    builder.disable_blending();
    builder.set_input_attribute(vertexDeclaration->layout, vertexDeclaration->stride);
    builder.set_multisampling_none();
    depthFormat != VK_FORMAT_UNDEFINED ?
      builder.enable_depth_test(true,
                                graphicsState.depth.writeEnable,
                                graphicsState.depth.compareOperation) :
      builder.enable_depth_test(false,
                                graphicsState.depth.writeEnable,
                                graphicsState.depth.compareOperation);
    builder.set_depth_format(depthFormat);
    builder.SetColorAttachmentFormats(attachmentFormats);

    pipeline.m_pipeline = builder.build(device);

    // TODO Temporal?
    pipeline.m_descriptorPoolSizes = globalPoolSizes;

    vkDestroyShaderModule(device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);

    return pipeline;
}

void VulkanPipeline::Destroy(VkDevice device)
{
    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);

    for (auto& descriptorSetLayout : m_descriptorSetLayouts)
    {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout.descriptorSetLayout, nullptr);
        descriptorSetLayout.bindings.clear();
    }
}

VkPipeline       VulkanPipeline::GetPipeline() const { return m_pipeline; }
VkPipelineLayout VulkanPipeline::GetPipelineLayout() const { return m_pipelineLayout; }
VulkanPipeline::DescriptorSetLayouts VulkanPipeline::GetDescriptorSetLayouts() const
{
    return m_descriptorSetLayouts;
}
const std::vector<VkDescriptorPoolSize>& VulkanPipeline::GetDescriptorPoolSizes() const
{
    return m_descriptorPoolSizes;
}

VkShaderModule VulkanPipeline::CreateShaderModule(VkDevice device, const char* filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        printf("[ERROR]: Could not open shader file %s", filename);
        return VK_NULL_HANDLE;
    }

    const size_t     file_size = static_cast<size_t>(file.tellg());
    std::vector<u32> buffer(file_size / sizeof(u32));

    file.seekg(0);
    file.read((char*)buffer.data(), file_size);
    file.close();

    VkShaderModuleCreateInfo info{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = buffer.size() * sizeof(u32),
      .pCode    = buffer.data(),
    };

    VkShaderModule module;
    if (vkCreateShaderModule(device, &info, nullptr, &module) != VK_SUCCESS)
    {
        printf("[ERROR]: Could not create shader module %s", filename);
        return VK_NULL_HANDLE;
    }

    return module;
}

void VulkanPipeline::AddDescriptorSetLayoutBindings(
  const Shader&                                                               shader,
  std::array<std::vector<VkDescriptorSetLayoutBinding>, MAX_DESCRIPTOR_SETS>& bindings)
{
    for (const auto& uniform : shader.uniformDataSlots)
    {
        assert(uniform.m_uniformType != UniformType::COUNT);

        VkShaderStageFlags shaderStage = shader.shaderType == ShaderType::VERTEX ?
                                           VK_SHADER_STAGE_VERTEX_BIT :
                                           VK_SHADER_STAGE_FRAGMENT_BIT;

        auto descriptorType = uniform.m_uniformType == UniformType::BUFFER ?
                                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER :
                                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

        VkDescriptorSetLayoutBinding binding{
          .binding            = static_cast<u32>(uniform.m_binding),
          .descriptorType     = descriptorType,
          .descriptorCount    = uniform.m_count,
          .stageFlags         = shaderStage,
          .pImmutableSamplers = nullptr,
        };
        bindings.at(uniform.m_set).emplace_back(std::move(binding));
    }
}

VulkanPipeline::DescriptorSetLayouts
VulkanPipeline::CreateDescriptorSetLayouts(VkDevice device, const RenderPipeline& pipeline)
{
    const auto bindingCount = pipeline.vertexShader.uniformDataSlots.size() +
                              pipeline.fragmentShader.uniformDataSlots.size();

    std::array<std::vector<VkDescriptorSetLayoutBinding>, MAX_DESCRIPTOR_SETS>
      descriptorSetLayoutBindings;

    for (auto& set : descriptorSetLayoutBindings)
    {
        set.reserve(MAX_UNIFORM_SLOTS);
    }

    AddDescriptorSetLayoutBindings(pipeline.vertexShader, descriptorSetLayoutBindings);
    AddDescriptorSetLayoutBindings(pipeline.fragmentShader, descriptorSetLayoutBindings);

    DescriptorSetLayouts descriptorSetLayouts{};

    u32 index = 0;
    for (auto& binding : descriptorSetLayoutBindings)
    {
        if (binding.empty())
            continue;

        VkDescriptorSetLayoutCreateInfo layoutInfo{
          .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
          .pNext        = nullptr,
          .bindingCount = static_cast<u32>(binding.size()),
          .pBindings    = binding.data(),
        };

        VkDescriptorSetLayout layout;
        auto ok = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &layout);
        assert(ok == VK_SUCCESS);

        DescriptorSetLayout descriptorSetLayout(std::move(layout), binding);
        descriptorSetLayouts.at(index++) = std::move(descriptorSetLayout);
    }

    return descriptorSetLayouts;
}
} // namespace vulkan
} // namespace RED
