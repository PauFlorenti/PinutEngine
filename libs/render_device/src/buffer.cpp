#include "pch.hpp"

#include "render_device/buffer.h"
#include "render_device/device.h"

namespace RED
{
GPUBuffer::GPUBuffer() = default;

GPUBuffer::GPUBuffer(BufferResource id, u64 size, std::weak_ptr<Device> device)
: m_id(id),
  m_size(size),
  m_device(device){};

GPUBuffer::~GPUBuffer() { Destroy(); }

void GPUBuffer::Destroy()
{
    if (const auto& device = m_device.lock())
    {
        device->DestroyBuffer(m_id);
    }
}

BufferResource GPUBuffer::GetID() const { return m_id; }

u64 GPUBuffer::GetSize() const { return m_size; }
} // namespace RED
