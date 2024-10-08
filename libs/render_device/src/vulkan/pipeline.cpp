#include "pch.hpp"

#include "src/renderPipeline.h"
#include "src/shader.h"
#include "src/states.h"
#include "src/vulkan/pipeline.h"
#include "src/vulkan/pipelineBuilder.h"
#include "src/vulkan/utils.h"

namespace vulkan
{
Pipeline::Pipeline()                = default;
Pipeline::Pipeline(const Pipeline&) = default;
Pipeline::~Pipeline()               = default;

Pipeline Pipeline::Create(VkDevice              device,
                          const RenderPipeline& renderPipeline,
                          const GraphicsState&  graphicsState)
{
    Pipeline pipeline;

    VkShaderModule vertexShaderModule, fragmentShaderModule;
    const auto     vertexShaderPath   = renderPipeline.vertexShader.name + ".spv";
    const auto     fragmentShaderPath = renderPipeline.fragmentShader.name + ".spv";
    vertexShaderModule                = CreateShaderModule(device, vertexShaderPath.c_str());
    fragmentShaderModule              = CreateShaderModule(device, fragmentShaderPath.c_str());

    if (!vertexShaderModule || !fragmentShaderModule)
    {
        printf("[ERROR]: Failed to create shaders.");
        return {};
    }

    // TODO should be provided
    // ! Hardcoded so far
    auto pipelineLayoutInfo = vkinit::PipelineLayoutCreateInfo(0, nullptr);
    assert(
      vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline.m_pipelineLayout) ==
      VK_SUCCESS);

    // TODO should be provided
    // ! Hardcoded so far
    auto inputAttribute =
      vkinit::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0);

    PipelineBuilder builder;
    builder.layout = pipeline.m_pipelineLayout;
    builder.set_shaders(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    builder.set_shaders(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.set_topology(graphicsState.topology);
    builder.set_rasterizer(graphicsState.raster);
    builder.disable_blending();
    // builder.setBlendingState(graphicsState.blend);
    builder.set_input_attribute({inputAttribute}, sizeof(f32) * 3);
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

void Pipeline::Destroy(VkDevice device)
{
    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}

VkPipeline       Pipeline::GetPipeline() const { return m_pipeline; }
VkPipelineLayout Pipeline::GetPipelineLayout() const { return m_pipelineLayout; }

VkShaderModule Pipeline::CreateShaderModule(VkDevice device, const char* filename)
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
} // namespace vulkan
