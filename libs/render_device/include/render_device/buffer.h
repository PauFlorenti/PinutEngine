#pragma once

#include "render_device/gpuresource.h"

namespace RED
{
class Device;
class GPUBuffer final
{
  public:
    GPUBuffer();
    GPUBuffer(BufferResource id, u64 size, Device* device);
    GPUBuffer(GPUBuffer&& other) noexcept;
    GPUBuffer& operator=(GPUBuffer&& other) noexcept;
    GPUBuffer(const GPUBuffer&)            = default;
    GPUBuffer& operator=(const GPUBuffer&) = default;
    ~GPUBuffer();

    void Destroy();

    BufferResource GetID() const;
    u64            GetSize() const;

  private:
    BufferResource m_id{GPU_RESOURCE_INVALID, ResourceType::BUFFER};
    u64            m_size{0}; // in bytes
    Device*        m_device{nullptr};
};

class GPUBufferView final
{
  public:
    GPUBufferView();
    GPUBufferView(const GPUBuffer& bufferOwner);
    GPUBufferView(BufferResource id, u64 size);
    GPUBufferView(const GPUBuffer& bufferOwner, u64 size);
    ~GPUBufferView();

    BufferResource GetID() const;
    u64            GetSize() const;
    bool           IsEmpty() const;

  private:
    BufferResource m_id{GPU_RESOURCE_INVALID, ResourceType::BUFFER};
    u64            m_size{0}; // in bytes
};
} // namespace RED
