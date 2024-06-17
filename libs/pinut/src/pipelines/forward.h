#pragma once

#include "src/core/buffer.h"
#include "src/core/gbuffer.h"

namespace Pinut
{
    class Device;
    class Scene;
    struct PerFrameData;

    class ForwardPass
    {
    public:
        void OnCreate(Device *inDevice);
        void OnDestroy();
        void OnCreateDisplayDependantResources(const uint32_t width, const uint32_t height);
        void OnDestroyDisplayDependantResources();

        // void Load
        void Draw(VkCommandBuffer cmd, const Scene *scene);
        void UpdatePerFrameData(VkCommandBuffer cmd, const PerFrameData &inData);
        const Texture &GetDepth() const { return depthTexture; }
        const VkPipeline GetPipeline() const { return pipeline; }

        VkDescriptorSetLayout GetPerObjectDescriptorSetLayout() const { return perObjectDescriptorSetLayout; }

    private:
        Device *device{nullptr};
        // GBuffer gbuffer;

        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        VkDescriptorSetLayout perFrameDescriptorSetLayout;
        VkDescriptorSetLayout perObjectDescriptorSetLayout;

        Texture depthTexture;
        Buffer perFrameBuffer;
    };
}
