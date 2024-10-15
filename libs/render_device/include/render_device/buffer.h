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
    ~GPUBuffer();

    void Destroy();

    BufferResource GetID() const;
    u64            GetSize() const;

  private:
    BufferResource m_id{GPU_RESOURCE_INVALID};
    u64            m_size{0}; // in bytes
    Device*        m_device{nullptr};
};

class GPUBufferView final
{
};
} // namespace RED
