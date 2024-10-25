#include "pch.hpp"

#include "render_device/renderPipeline.h"
#include "render_device/states.h"
#include "src/vulkan/pipelineBuilder.h"
#include "src/vulkan/utils.h"
#include "src/vulkan/vulkanPipeline.h"
#include "src/vulkan/vulkanVertexDeclaration.h"

namespace RED
{
namespace vulkan
{
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
    fragmentShaderModule              = CreateShaderModule(device, fragmentShaderPath.c_str());

    if (!vertexShaderModule || !fragmentShaderModule)
    {
        printf("[ERROR]: Failed to create shaders.");
        return {};
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

    PipelineBuilder builder;
    builder.layout = pipeline.m_pipelineLayout;
    builder.set_shaders(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    builder.set_shaders(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.set_topology(graphicsState.topology);
    builder.set_rasterizer(graphicsState.raster);
    builder.disable_blending();
    // builder.setBlendingState(graphicsState.blend);
    builder.set_input_attribute(vertexDeclaration->layout, vertexDeclaration->stride);
    builder.set_multisampling_none();
    builder.enable_depth_test(false, false, VK_COMPARE_OP_LESS_OR_EQUAL);
    // builder.set_depth_format(VK_FORMAT_D32_SFLOAT);
    // builder.set_stencil_format(VK_FORMAT_UNDEFINED);
    builder.set_color_attachment_format(VK_FORMAT_B8G8R8A8_UNORM);

    pipeline.m_pipeline = builder.build(device);

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
        VkShaderStageFlags shaderStage = shader.shaderType == ShaderType::VERTEX ?
                                           VK_SHADER_STAGE_VERTEX_BIT :
                                           VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding binding{
          .binding            = static_cast<u32>(uniform.m_binding),
          .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount    = 1,
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
