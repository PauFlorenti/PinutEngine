#pragma once

#include "render_device/device.h"
#include "render_device/drawCall.h"
#include "render_device/states.h"
#include "src/resourceGenerator.h"
#include "src/vulkan/descriptorSetManager.h"
#include "src/vulkan/vulkanBuffer.h"
#include "src/vulkan/vulkanPipeline.h"
#include "src/vulkan/vulkanTexture.h"

namespace RED
{
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

struct SwapchainState
{
    VkSwapchainKHR swapchain;
    VkImage        swapchainImage;
    u32            swapchainImageIndex;
    VkSemaphore    endFrameSemaphore;
};

using BeginFrameCallback_fn        = std::function<void(void*, VkSemaphore)>;
using EndFrameCallback_fn          = std::function<void(void*, VkSemaphore)>;
using GetSwapchainStateCallback_fn = std::function<SwapchainState()>;

struct DeviceCallbacks
{
    void*                        context;
    BeginFrameCallback_fn        BeginFrame_fn;
    EndFrameCallback_fn          EndFrame_fn;
    GetSwapchainStateCallback_fn GetSwapchainState_fn;
};

static constexpr u32 MAX_FRAMES_IN_FLIGHT = 3;
constexpr u32        UNIFORM_BUFFER_SIZE  = 1024; // Currently just 1MB

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
    ~VulkanDevice() override;

    void BeginFrame() override;
    void EndFrame() override;

    void EnableRendering(const VkRect2D&                               renderArea,
                         const std::vector<VkRenderingAttachmentInfo>& colorAttachments,
                         VkRenderingAttachmentInfo* depthAttachment = nullptr) override;
    void DisableRendering() override;

    void SetGraphicsState(GraphicsState* state) override;
    void SetRenderPipeline(const RenderPipeline* pipeline) override;

    void SubmitDrawCalls(const std::vector<DrawCall>& drawCalls) override;

    GPUBuffer CreateBuffer(const BufferDescriptor& descriptor, void* data = nullptr) override;
    void      UpdateBuffer(BufferResource bufferId, void* data) override;
    void      DestroyBuffer(BufferResource) override;

    GPUTexture CreateTexture(const TextureDescriptor& descriptor,
                             const void*              data = nullptr) override;
    void       DestroyTexture(TextureResource) override;

    VkRenderingAttachmentInfo GetAttachment(const GPUTextureView& textureView,
                                            VkImageLayout         layout,
                                            VkAttachmentLoadOp    loadOp,
                                            VkAttachmentStoreOp   storeOp,
                                            VkClearValue          clearValue) override;

    void TransitionImageLayout(TextureResource         image,
                               VkAccessFlags           srcAccessFlags,
                               VkAccessFlags           dstAccessFlags,
                               VkImageLayout           currentLayout,
                               VkImageLayout           targetLayout,
                               VkPipelineStageFlags    srcStageFlags,
                               VkPipelineStageFlags    dstStageFlags,
                               VkImageSubresourceRange subresourceRange,
                               bool                    immediate = false) override;

    void WaitIdle() const override;

    void BeginCommandRecording(QueueType type) override;
    void EndCommandRecording(bool waitForImage = true, bool signalFence = false) override;

  private:
    UniformDescriptorSetInfos GetUniformDescriptorSetInfos(
      const std::vector<UniformDescriptor>& uniformDescriptors);

    VkCommandBuffer BeginImmediateCommandBuffer(VkCommandPool commandPool);
    void            FlushImmediateCommandBuffer(VkCommandBuffer cmd,
                                                VkQueue         queue,
                                                VkFence         fence = VK_NULL_HANDLE) const;

    VkFence     CreateFence(VkFenceCreateFlags flags = 0x0) const;
    VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags = 0x0) const;

    VkCommandPool   CreateCommandPool(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags = 0x0);
    VkCommandBuffer CreateCommandBuffer(const VkCommandPool& commandPool);
    VulkanBuffer    CreateInternalBuffer(u32 size, VkBufferUsageFlagBits usage);

    VulkanPipeline* GetRenderPipeline(const RenderPipeline* pipeline,
                                      const GraphicsState&  graphicsState);
    VulkanBuffer    GetVulkanBuffer(const BufferResource& bufferResource);
    VulkanTexture   GetVulkanTexture(const TextureResource& textureResource);

    VulkanBuffer GetStagingBuffer(u64 size);
    void         TransitionImageLayout(VkImage                 image,
                                       VkAccessFlags           srcAccessFlags,
                                       VkAccessFlags           dstAccessFlags,
                                       VkImageLayout           currentLayout,
                                       VkImageLayout           targetLayout,
                                       VkPipelineStageFlags    srcStageFlags,
                                       VkPipelineStageFlags    dstStageFlags,
                                       VkImageSubresourceRange subresourceRange,
                                       bool                    immediate = false);

    void SubmitDrawCall(const DrawCall& drawCall);

    void UpdateBuffers();
    void UpdateTextures();

    VkDevice         m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkCommandPool    m_immediateCommandPool{VK_NULL_HANDLE};
    VmaAllocator     m_allocator{nullptr};

    u32            m_currentIndexFrame{0};
    CommandBuffer* m_currentCommandBuffer{nullptr};
    CommandBuffer* m_lastCommandBuffer{nullptr};

    void*                        m_rendererContext{nullptr};
    BeginFrameCallback_fn        m_beginFrame_fn;
    EndFrameCallback_fn          m_endFrame_fn;
    GetSwapchainStateCallback_fn m_getSwapchainState_fn;

    // 1 Graphics, 2 Compute
    std::array<CommandBufferSetData, 2> m_commandBufferSets;
    std::array<QueueInfo, 2>            m_queues;

    std::unordered_map<PipelineKey, VulkanPipeline> m_pipelines;
    const RenderPipeline*                           m_currentRenderPipeline{nullptr};
    VulkanPipeline*                                 m_currentRenderPipelineInternal{nullptr};
    GraphicsState                                   m_currentGraphicsState;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugMessenger{nullptr};
#endif

    // Resources
    DescriptorSetManager                                 m_descriptorSetManager;
    ResourceGenerator                                    m_resourceGenerator;
    std::unordered_map<BufferResource, VulkanBuffer>     m_buffers;
    std::unordered_map<TextureResource, VulkanTexture>   m_textures;
    VkSampler                                            m_sampler; // TODO Temporal
    std::unordered_map<u64, std::deque<VulkanBuffer>>    m_stagingBuffers;
    std::deque<VulkanBuffer>                             m_stagingBufferUsed;
    std::vector<std::pair<BufferResource, VulkanBuffer>> m_bufferToUpdate;

    std::array<VkFence, MAX_FRAMES_IN_FLIGHT>     m_frameCompletedFences;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imagesAvailableSemaphores;

    // Light uniform
    std::array<VulkanBuffer, MAX_FRAMES_IN_FLIGHT> m_globalUniformBuffers;
};
} // namespace vulkan
} // namespace RED
