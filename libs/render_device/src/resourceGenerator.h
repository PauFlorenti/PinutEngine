#pragma once

#include "render_device/gpuresource.h"

namespace RED
{
class ResourceGenerator
{
  public:
    BufferResource  GenerateBufferResource();
    TextureResource GenerateTextureResource();

  private:
    std::atomic<u64> m_buffer_id{0};
    std::atomic<u64> m_texture_id{0};
};
} // namespace RED
