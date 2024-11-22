#pragma once

#include "render_device/buffer.h"
#include "render_device/gpuresource.h"
#include "render_device/texture.h"

// RED = Rendering Engine Device
namespace RED
{
struct BufferDescriptor;
struct DrawCall;
struct GraphicsState;
struct RenderPipeline;
struct TextureDescriptor;

enum class QueueType
{
    GRAPHICS,
    COMPUTE,
    COUNT
};
class Device
{
  public:
    static std::shared_ptr<Device> Create(void* device, void* queues, void* callbacks);

    Device();
    Device(const Device&)            = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&)                 = delete;
    Device& operator=(Device&&)      = delete;

    virtual ~Device() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame()   = 0;

    virtual void EnableRendering(const VkRect2D&                               renderArea,
                                 const std::vector<VkRenderingAttachmentInfo>& colorAttachments,
                                 VkRenderingAttachmentInfo* depthAttachment = nullptr) = 0;
    virtual void DisableRendering()                                                    = 0;

    virtual void SetGraphicsState(GraphicsState* state)      = 0;
    virtual void SetRenderPipeline(RenderPipeline* pipeline) = 0;

    virtual void SubmitDrawCalls(const std::vector<DrawCall>& drawCalls) = 0;

    virtual GPUBuffer CreateBuffer(const BufferDescriptor&, void* data = nullptr) = 0;
    virtual void      UpdateBuffer(BufferResource bufferId, void* data)           = 0;
    virtual void      DestroyBuffer(BufferResource)                               = 0;

    virtual GPUTexture CreateTexture(const TextureDescriptor& descriptor, void* data = nullptr) = 0;
    virtual void       DestroyTexture(TextureResource)                                          = 0;

    virtual VkRenderingAttachmentInfo GetAttachment(const GPUTextureView& textureView,
                                                    VkImageLayout         layout,
                                                    VkAttachmentLoadOp    loadOp,
                                                    VkAttachmentStoreOp   storeOp,
                                                    VkClearValue          clearValue) = 0;

    virtual void TransitionImageLayout(TextureResource         image,
                                       VkAccessFlags           srcAccessFlags,
                                       VkAccessFlags           dstAccessFlags,
                                       VkImageLayout           currentLayout,
                                       VkImageLayout           targetLayout,
                                       VkPipelineStageFlags    srcStageFlags,
                                       VkPipelineStageFlags    dstStageFlags,
                                       VkImageSubresourceRange subresourceRange,
                                       bool                    immediate = false) = 0;

    virtual void WaitIdle() const = 0;

    virtual void BeginCommandRecording(QueueType type)                                   = 0;
    virtual void EndCommandRecording(bool waitForImage = true, bool signalFence = false) = 0;
};

} // namespace RED
