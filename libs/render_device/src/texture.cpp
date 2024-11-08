#include "pch.hpp"

#include "render_device/device.h"
#include "render_device/texture.h"

namespace RED
{
GPUTexture::GPUTexture() = default;

GPUTexture::GPUTexture(TextureResource id, Device* device) : m_id(id), m_device(device) {};

GPUTexture::GPUTexture(GPUTexture&& other) noexcept { *this = std::move(other); }

GPUTexture& GPUTexture::operator=(GPUTexture&& other) noexcept
{
    Destroy();

    m_id     = other.m_id;
    m_device = std::move(other.m_device);

    other.m_device = nullptr;

    return *this;
}

GPUTexture::~GPUTexture() { Destroy(); }

void GPUTexture::Destroy()
{
    if (!m_device)
        return;

    m_device->DestroyTexture(m_id);
    m_id     = {};
    m_device = nullptr;
}

TextureResource GPUTexture::GetID() const { return m_id; }

bool GPUTexture::IsEmpty() const { return m_id.id == GPU_RESOURCE_INVALID; }

GPUTextureView::GPUTextureView() = default;

GPUTextureView::GPUTextureView(const GPUTexture& textureOwner) : m_id(textureOwner.GetID()) {}

GPUTextureView::GPUTextureView(TextureResource id) : m_id(id) {}

GPUTextureView::~GPUTextureView() {}

TextureResource GPUTextureView::GetID() const { return m_id; }

bool GPUTextureView::IsEmpty() const { return m_id.id == GPU_RESOURCE_INVALID; }

} // namespace RED
