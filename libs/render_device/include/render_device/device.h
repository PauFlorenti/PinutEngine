#pragma once

#include "render_device/buffer.h"
#include "render_device/gpuresource.h"

// RED = Rendering Engine Device
namespace RED
{
struct BufferDescriptor;
struct DrawCall;
struct GraphicsState;
struct RenderPipeline;
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

    virtual void OnDestroy() = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame()   = 0;
    virtual void Present()    = 0;

    virtual void EnableRendering(const VkRect2D&                               renderArea,
                                 const std::vector<VkRenderingAttachmentInfo>& attachments) = 0;
    virtual void DisableRendering()                                                         = 0;

    virtual void SetGraphicsState(GraphicsState* state)      = 0;
    virtual void SetRenderPipeline(RenderPipeline* pipeline) = 0;

    virtual void SubmitDrawCalls(const std::vector<DrawCall>& drawCalls) = 0;

    virtual GPUBuffer CreateBuffer(const BufferDescriptor&, void* data) = 0;
    virtual void      DestroyBuffer(BufferResource)                     = 0;

    virtual void TransitionImageLayout(VkImage                 image,
                                       VkAccessFlags           srcAccessFlags,
                                       VkAccessFlags           dstAccessFlags,
                                       VkImageLayout           currentLayout,
                                       VkImageLayout           targetLayout,
                                       VkPipelineStageFlags    srcStageFlags,
                                       VkPipelineStageFlags    dstStageFlags,
                                       VkImageSubresourceRange subresourceRange) = 0;

    virtual void WaitIdle() const = 0;
};

} // namespace RED
