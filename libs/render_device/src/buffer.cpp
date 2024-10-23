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

GPUBuffer::GPUBuffer(GPUBuffer&& other) noexcept { *this = std::move(other); }

GPUBuffer& GPUBuffer::operator=(GPUBuffer&& other) noexcept
{
    Destroy();

    m_id     = other.m_id;
    m_size   = other.m_size;
    m_device = std::move(other.m_device);

    other.m_size   = 0;
    other.m_device = nullptr;

    return *this;
}

GPUBuffer::~GPUBuffer() { Destroy(); }

void GPUBuffer::Destroy()
{
    m_id   = {};
    m_size = 0;

    if (!m_device)
        return;

    m_device->DestroyBuffer(m_id);
    m_device = nullptr;
}

BufferResource GPUBuffer::GetID() const { return m_id; }

u64 GPUBuffer::GetSize() const { return m_size; }

GPUBufferView::GPUBufferView() = default;

GPUBufferView::GPUBufferView(const GPUBuffer& bufferOwner)
: m_id(bufferOwner.GetID()),
  m_size(bufferOwner.GetSize())
{
}

GPUBufferView::GPUBufferView(BufferResource id, u64 size) : m_id(id), m_size(size) {}

GPUBufferView::GPUBufferView(const GPUBuffer& bufferOwner, u64 size)
: m_id(bufferOwner.GetID()),
  m_size(size)
{
}

GPUBufferView::~GPUBufferView() {}

BufferResource GPUBufferView::GetID() const { return m_id; }

u64 GPUBufferView::GetSize() const { return m_size; }

bool GPUBufferView::IsEmpty() const { return m_id.id == GPU_RESOURCE_INVALID; }

} // namespace RED
