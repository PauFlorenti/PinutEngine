#include "stdafx.h"

#include "src/assets/mesh.h"
#include "src/assets/texture.h"
#include "src/renderer/descriptorSetManager.h"
#include "src/renderer/materials/skyboxMaterial.h"
#include "src/renderer/pipeline.h"
#include "src/renderer/utils.h"

namespace Pinut
{
void SkyboxMaterial::BuildPipeline(VkDevice device)
{
    VkShaderModule vertex_shader;
    if (!vkinit::load_shader_module("shaders/skybox.vert.spv", device, &vertex_shader))
    {
        printf("[ERROR]: Error building the forward vertex shader.");
    }

    VkShaderModule fragment_shader;
    if (!vkinit::load_shader_module("shaders/skybox.frag.spv", device, &fragment_shader))
    {
        printf("[ERROR]: Error building the forward fragment shader.");
    }

    // clang-format off
    std::vector<VkVertexInputAttributeDescription> input_attributes{
        vkinit::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)),
        vkinit::VertexInputAttributeDescription(1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)),
    };
    // clang-format on

    VkDescriptorSetLayoutBinding perFrameBindings =
      vkinit::DescriptorSetLayoutBinding(0,
                                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         1,
                                         VK_SHADER_STAGE_VERTEX_BIT);

    auto perFrameDescriptorSetLayoutCreateInfo =
      vkinit::DescriptorSetLayoutCreateInfo(1, &perFrameBindings);

    vkCreateDescriptorSetLayout(device,
                                &perFrameDescriptorSetLayoutCreateInfo,
                                nullptr,
                                &m_perFrameDescriptorSetLayout);

    VkDescriptorSetLayoutBinding perObjectBindings[1] = {
      vkinit::DescriptorSetLayoutBinding(0,
                                         VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                         1,
                                         VK_SHADER_STAGE_FRAGMENT_BIT),
    };

    auto perObjectDescriptorSetLayoutCreateInfo =
      vkinit::DescriptorSetLayoutCreateInfo(1, perObjectBindings);

    vkCreateDescriptorSetLayout(device,
                                &perObjectDescriptorSetLayoutCreateInfo,
                                nullptr,
                                &m_perObjectDescriptorSetLayout);

    std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {m_perFrameDescriptorSetLayout,
                                                                 m_perObjectDescriptorSetLayout};
    auto layoutInfo = vkinit::PipelineLayoutCreateInfo(2, descriptorSetLayouts.data());

    VkPushConstantRange pushConstantRange{
      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
      .offset     = 0,
      .size       = sizeof(glm::mat4),
    };

    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges    = &pushConstantRange;

    auto ok = vkCreatePipelineLayout(device, &layoutInfo, nullptr, &m_pipelineLayout);
    assert(ok == VK_SUCCESS);

    PipelineBuilder builder;
    builder.layout = m_pipelineLayout;
    builder.set_shaders(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
    builder.set_shaders(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);
    builder.set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    builder.set_rasterizer(VK_POLYGON_MODE_FILL,
                           VK_CULL_MODE_NONE,
                           VK_FRONT_FACE_COUNTER_CLOCKWISE);
    builder.set_input_attribute(std::move(input_attributes), sizeof(Vertex));
    builder.set_multisampling_none();
    builder.enable_skybox_blending();
    builder.enable_depth_test(false, false);
    builder.set_depth_format(VK_FORMAT_D32_SFLOAT);
    builder.set_stencil_format(VK_FORMAT_UNDEFINED);
    builder.set_color_attachment_format(VK_FORMAT_B8G8R8A8_UNORM);

    m_pipeline = builder.build(device);

    vkDestroyShaderModule(device, vertex_shader, nullptr);
    vkDestroyShaderModule(device, fragment_shader, nullptr);
}

void SkyboxMaterial::Destroy(VkDevice device)
{
    vkDestroyDescriptorSetLayout(device, m_perFrameDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_perObjectDescriptorSetLayout, nullptr);

    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}
} // namespace Pinut
