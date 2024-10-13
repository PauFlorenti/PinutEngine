#pragma once

#include "render_device/device.h"
#include "render_device/states.h"
#include "src/vulkan/pipeline.h"

typedef struct VmaAllocator_T* VmaAllocator;

namespace RED
{
struct DrawCall;
struct RenderPipeline;
namespace vulkan
{
struct DeviceInfo
{
    VkInstance       instance;
    VkDevice         device;
    VkPhysicalDevice physicalDevice;
};

struct QueueInfo
{
    VkQueue queue{VK_NULL_HANDLE};
    u32     index;
};

using BeginFrameCallback_fn = std::function<void(void*, VkSemaphore)>;
using EndFrameCallback_fn   = std::function<void(void*, VkSemaphore)>;

struct DeviceCallbacks
{
    void*                         context;
    vulkan::BeginFrameCallback_fn BeginFrame_fn;
    vulkan::EndFrameCallback_fn   EndFrame_fn;
};

enum class QueueType
{
    GRAPHICS,
    COMPUTE,
    COUNT
};

static constexpr u32 MAX_FRAMES_IN_FLIGHT = 3;

class VulkanDevice final : public Device
{
  private:
    struct CommandBuffer
    {
        QueueType       queueType;
        VkCommandBuffer commandBuffer;
        VkSemaphore     signalSemaphore;
    };

    struct CommandBufferSetData
    {
        std::array<VkCommandPool, MAX_FRAMES_IN_FLIGHT>               commandPools;
        std::array<std::vector<CommandBuffer*>, MAX_FRAMES_IN_FLIGHT> commandBuffers;
        std::array<i32, MAX_FRAMES_IN_FLIGHT>                         commandBufferIndex;
    };

  public:
    VulkanDevice(void* deviceInfo, void* queues, void* callbacks);
    ~VulkanDevice() override = default;

    void OnDestroy();

    void BeginFrame();
    void EndFrame();

    void EnableRendering(const VkRect2D&                               renderArea,
                         const std::vector<VkRenderingAttachmentInfo>& attachments);
    void DisableRendering();

    void SetGraphicsState(GraphicsState* state);
    void SetRenderPipeline(RenderPipeline* pipeline);

    void SubmitDrawCalls(const std::vector<DrawCall>& drawCalls);

    void TransitionImageLayout(VkImage                 image,
                               VkAccessFlags           srcAccessFlags,
                               VkAccessFlags           dstAccessFlags,
                               VkImageLayout           currentLayout,
                               VkImageLayout           targetLayout,
                               VkPipelineStageFlags    srcStageFlags,
                               VkPipelineStageFlags    dstStageFlags,
                               VkImageSubresourceRange subresourceRange);

    void WaitIdle() const;

  private:
    void            BeginCommandRecording(QueueType type);
    void            EndCommandRecording(bool waitForImage = true, bool signalFence = false);
    VkCommandBuffer BeginImmediateCommandBuffer(VkCommandPool commandPool);
    void            FlushImmediateCommandBuffer(VkCommandBuffer cmd,
                                                VkQueue         queue,
                                                VkFence         fence = VK_NULL_HANDLE) const;

    VkFence     CreateFence(VkFenceCreateFlags flags = 0x0) const;
    VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags = 0x0) const;

    VkCommandPool   CreateCommandPool(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags = 0x0);
    VkCommandBuffer CreateCommandBuffer(const VkCommandPool& commandPool);

    Pipeline* GetRenderPipeline(const RenderPipeline* pipeline, const GraphicsState& graphicsState);

    void SubmitDrawCall(const DrawCall& drawCall);

    VkDevice         m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkCommandPool    m_immediateCommandPool{VK_NULL_HANDLE};
    VmaAllocator     m_allocator{nullptr};

    u32            m_currentIndexFrame{0};
    CommandBuffer* m_currentCommandBuffer{nullptr};
    CommandBuffer* m_lastCommandBuffer{nullptr};

    void*                 m_rendererContext{nullptr};
    BeginFrameCallback_fn m_beginFrame_fn;
    EndFrameCallback_fn   m_endFrame_fn;

    // 1 Graphics, 2 Compute
    std::array<CommandBufferSetData, 2> m_commandBufferSets;
    std::array<QueueInfo, 2>            m_queues;

    std::unordered_map<PipelineKey, Pipeline> m_pipelines;
    const RenderPipeline*                     m_currentRenderPipeline{nullptr};
    Pipeline*                                 m_currentRenderPipelineInternal{nullptr};
    GraphicsState                             m_currentGraphicsState;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugMessenger{nullptr};
#endif

    std::array<VkFence, MAX_FRAMES_IN_FLIGHT>     m_frameCompletedFences;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imagesAvailableSemaphores;
};
} // namespace vulkan
} // namespace RED
