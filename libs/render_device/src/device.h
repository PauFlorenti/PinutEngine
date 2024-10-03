#pragma once

typedef struct VmaAllocator_T* VmaAllocator;

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

class Device final
{
  private:
    struct CommandBufferSetData
    {
        std::array<VkCommandPool, MAX_FRAMES_IN_FLIGHT> commandPools;
        std::array<i32, MAX_FRAMES_IN_FLIGHT>           commandBufferIndex;
    };

  public:
    Device(DeviceInfo* deviceInfo, QueueInfo* queues, DeviceCallbacks* callbacks);
    ~Device() = default;

    void OnDestroy();

    void BeginFrame();
    void EndFrame();

    VkCommandBuffer CreateImmediateCommandBuffer();
    void            FlushCommandBuffer(VkCommandBuffer cmd) const;

    void WaitIdle() const;

  private:
    VkFence     CreateFence(VkFenceCreateFlags flags = 0x0) const;
    VkSemaphore CreateSemaphore(VkSemaphoreCreateFlags flags = 0x0) const;

    VkCommandPool   CreateCommandPool(u32 queueFamilyIndex, VkCommandPoolCreateFlags flags = 0x0);
    VkCommandBuffer CreateCommandBuffer(const VkCommandPool& commandPool);

    VkDevice         m_device{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkCommandPool    m_commandPool{VK_NULL_HANDLE};
    VmaAllocator     m_allocator{nullptr};

    u32 m_currentIndexFrame{0};

    void*                 m_rendererContext{nullptr};
    BeginFrameCallback_fn m_beginFrame_fn;
    EndFrameCallback_fn   m_endFrame_fn;

    // 1 Graphics, 2 Compute
    std::array<CommandBufferSetData, 2> m_commandBufferSets;
    std::array<QueueInfo, 2>            m_queues;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debugMessenger{nullptr};
#endif

    std::array<VkFence, MAX_FRAMES_IN_FLIGHT>     m_frameCompletedFences;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imagesAvailableSemaphores;
};
} // namespace vulkan
