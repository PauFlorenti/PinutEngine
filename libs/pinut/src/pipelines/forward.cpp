// Some comment

#include "stdafx.h"

#include "forward.h"
#include "pipelineBuilder.h"
#include "src/core/buffer.h"
#include "src/core/common.h"
#include "src/core/device.h"
#include "src/core/texture.h"
#include "src/scene/material.h"
#include "src/scene/mesh.h"
#include "src/scene/scene.h"

static bool load_shader_module(const char *filename, VkDevice device, VkShaderModule *out_shader_module)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        return false;

    const size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));

    file.seekg(0);
    file.read((char *)buffer.data(), file_size);
    file.close();

    VkShaderModuleCreateInfo info{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = buffer.size() * sizeof(uint32_t),
        .pCode = buffer.data(),
    };

    VkShaderModule module;
    if (vkCreateShaderModule(device, &info, nullptr, &module) != VK_SUCCESS)
        return false;

    *out_shader_module = module;
    return true;
}

namespace Pinut
{
    void ForwardPass::OnCreate(Device *inDevice)
    {
        device = inDevice;
        const auto logicalDevice = device->GetDevice();

        VkShaderModule vertex_shader;
        if (!load_shader_module("shaders/forward.vert.spv", logicalDevice, &vertex_shader))
        {
            printf("[ERROR]: Error building the forward vertex shader.");
        }

        VkShaderModule fragment_shader;
        if (!load_shader_module("shaders/forward.frag.spv", logicalDevice, &fragment_shader))
        {
            printf("[ERROR]: Error building the forward fragment shader.");
        }

        std::vector<VkVertexInputAttributeDescription> input_attributes{
            vkinit::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)),
            vkinit::VertexInputAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)),
            vkinit::VertexInputAttributeDescription(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vertex, color)),
            vkinit::VertexInputAttributeDescription(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)),
        };

        auto perFrameBinding = vkinit::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        auto perFrameDescriptorSetLayoutInfo = vkinit::DescriptorSetLayoutCreateInfo(1, &perFrameBinding);

        vkCreateDescriptorSetLayout(logicalDevice, &perFrameDescriptorSetLayoutInfo, nullptr, &perFrameDescriptorSetLayout);

        auto perObjectBinding = vkinit::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
        auto perObjectTextureBinding = vkinit::DescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, VK_SHADER_STAGE_FRAGMENT_BIT);

        VkDescriptorSetLayoutBinding bindings[2] = {perObjectBinding, perObjectTextureBinding};
        auto perObjectDescriptorSetLayoutInfo = vkinit::DescriptorSetLayoutCreateInfo(2, bindings);

        vkCreateDescriptorSetLayout(logicalDevice, &perObjectDescriptorSetLayoutInfo, nullptr, &perObjectDescriptorSetLayout);

        VkDescriptorSetLayout layouts[2] = {perFrameDescriptorSetLayout, perObjectDescriptorSetLayout};
        auto layout_info = vkinit::PipelineLayoutCreateInfo(2, layouts);

        auto ok = vkCreatePipelineLayout(logicalDevice, &layout_info, nullptr, &pipelineLayout);
        assert(ok == VK_SUCCESS);

        PipelineBuilder builder;
        builder.layout = pipelineLayout;
        builder.set_shaders(vertex_shader, VK_SHADER_STAGE_VERTEX_BIT);
        builder.set_shaders(fragment_shader, VK_SHADER_STAGE_FRAGMENT_BIT);
        builder.set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        builder.set_input_attribute(std::move(input_attributes), sizeof(Vertex));
        builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
        builder.set_cull_mode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
        builder.set_multisampling_none();
        builder.disable_blending();
        builder.enable_depth_test(true, VK_COMPARE_OP_LESS);
        builder.set_depth_format(VK_FORMAT_D32_SFLOAT_S8_UINT);

        pipeline = builder.build(logicalDevice);

        perFrameBuffer.Create(device, sizeof(PerFrameData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

        vkDestroyShaderModule(logicalDevice, vertex_shader, nullptr);
        vkDestroyShaderModule(logicalDevice, fragment_shader, nullptr);
    }

    void ForwardPass::OnDestroy()
    {
        const auto localDevice = device->GetDevice();

        perFrameBuffer.Destroy();
        vkDestroyDescriptorSetLayout(localDevice, perFrameDescriptorSetLayout, nullptr);
        vkDestroyDescriptorSetLayout(localDevice, perObjectDescriptorSetLayout, nullptr);

        OnDestroyDisplayDependantResources();

        vkDestroyPipeline(localDevice, pipeline, nullptr);
        vkDestroyPipelineLayout(localDevice, pipelineLayout, nullptr);
    }

    void ForwardPass::OnCreateDisplayDependantResources(const uint32_t width, uint32_t height)
    {
        depthTexture.CreateRenderTarget(device, width, height, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, "DepthTexture");
    }

    void ForwardPass::OnDestroyDisplayDependantResources()
    {
        depthTexture.Destroy();
    }

    void ForwardPass::Draw(VkCommandBuffer cmd, const Scene *scene)
    {
        const auto logicalDevice = device->GetDevice();
        const auto frameIndex = device->GetFrameIndex();
        const auto &descriptorSetManager = device->GetDescriptorSetManager();

        for (auto &r : scene->GetRenderables())
        {
            VkDescriptorSet set = descriptorSetManager.Allocate(perObjectDescriptorSetLayout);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &set, 0, nullptr);

            auto renderableBuffer = r->GetBuffer();
            auto data = (PerObjectData *)renderableBuffer.allocationInfo.pMappedData;
            data->model = r->GetModelMatrix();
            data->inverse_model = glm::inverse(r->GetModelMatrix());

            auto bufferInfo = vkinit::DescriptorBufferInfo(renderableBuffer.buffer, 0, VK_WHOLE_SIZE);

            const auto material = r->GetMaterial();
            const auto albedo = material->GetAlbedo();
            const auto normal = material->GetNormal();
            const auto roughness = material->GetRoughness();

            VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            auto diffuseImage = vkinit::DescriptorImageInfo(albedo->GetImageView(), albedo->GetSampler(), imageLayout);
            auto normalImage = vkinit::DescriptorImageInfo(normal->GetImageView(), normal->GetSampler(), imageLayout);
            auto roughnessImage = vkinit::DescriptorImageInfo(roughness->GetImageView(), roughness->GetSampler(), imageLayout);

            auto bufferWrite = vkinit::WriteDescriptorSet(set, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo);

            VkDescriptorImageInfo imageInfos[3] = {diffuseImage, normalImage, roughnessImage};
            auto imageWrite = vkinit::WriteDescriptorSet(set, 1, 3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, nullptr, imageInfos);

            VkWriteDescriptorSet writes[2] = {bufferWrite, imageWrite};
            vkUpdateDescriptorSets(logicalDevice, 2, writes, 0, nullptr);

            r->Draw(cmd);
        }
    }

    void ForwardPass::UpdatePerFrameData(VkCommandBuffer cmd, const PerFrameData &inData)
    {
        auto set = device->GetDescriptorSetManager().Allocate(perFrameDescriptorSetLayout);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &set, 0, nullptr);

        auto data = (PerFrameData *)perFrameBuffer.allocationInfo.pMappedData;
        *data = inData;

        auto bufferInfo = vkinit::DescriptorBufferInfo(perFrameBuffer.buffer, 0, VK_WHOLE_SIZE);
        VkWriteDescriptorSet write = vkinit::WriteDescriptorSet(set, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, &bufferInfo);
        vkUpdateDescriptorSets(device->GetDevice(), 1, &write, 0, nullptr);
    }
}
