#include "pch.hpp"

#include "render_device/buffer.h"
#include "render_device/device.h"

namespace RED
{
GPUBuffer::GPUBuffer() = default;

GPUBuffer::GPUBuffer(BufferResource id, u64 size, Device* device)
: m_id(id),
  m_size(size),
  m_device(device){};

GPUBuffer::~GPUBuffer() = default; //{ Destroy(); }

void GPUBuffer::Destroy()
{
    assert(m_device);
    m_device->DestroyBuffer(m_id);
    m_id   = {};
    m_size = 0;
}

BufferResource GPUBuffer::GetID() const { return m_id; }

u64 GPUBuffer::GetSize() const { return m_size; }
} // namespace RED
