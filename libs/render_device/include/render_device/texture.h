#pragma once

#include "render_device/gpuresource.h"

namespace RED
{
class Device;
class GPUTexture final
{
  public:
    GPUTexture();
    GPUTexture(TextureResource id, Device* device);
    GPUTexture(GPUTexture&& other) noexcept;
    GPUTexture& operator=(GPUTexture&& other) noexcept;
    GPUTexture(const GPUTexture&)            = delete;
    GPUTexture& operator=(const GPUTexture&) = delete;
    ~GPUTexture();

    void Destroy();

    TextureResource GetID() const;

  private:
    TextureResource m_id{GPU_RESOURCE_INVALID, ResourceType::BUFFER};
    Device*         m_device{nullptr};
};

class GPUTextureView final
{
  public:
    GPUTextureView();
    GPUTextureView(const GPUTexture& textureOwner);
    GPUTextureView(TextureResource id);
    ~GPUTextureView();

    TextureResource GetID() const;
    bool            IsEmpty() const;

  private:
    TextureResource m_id{GPU_RESOURCE_INVALID, ResourceType::BUFFER};
};
} // namespace RED
