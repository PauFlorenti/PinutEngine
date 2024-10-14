#pragma once

#include "render_device/gpuresource.h"

namespace RED
{
class Device;
class GPUBuffer final
{
  public:
    GPUBuffer();
    GPUBuffer(BufferResource id, u64 size, std::weak_ptr<Device> device);
    ~GPUBuffer();

    void Destroy();

    BufferResource GetID() const;
    u64            GetSize() const;

  private:
    BufferResource        m_id;
    u64                   m_size; // in bytes
    std::weak_ptr<Device> m_device;
};

class GPUBufferView final
{
};
} // namespace RED
