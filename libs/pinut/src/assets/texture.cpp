#include "stdafx.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <external/stb/stb_image.h>
#include <external/stb/stb_image_write.h>

#include "texture.h"

namespace Pinut
{
Texture::Texture(const u32 width, const u32 height, VkFormat format, void* data)
: m_width(width),
  m_height(height),
  m_format(format),
  m_data(data)
{
}

Texture::Texture(const std::filesystem::path& filepath)
{
    const auto absolutePath = std::filesystem::absolute(filepath).string();
    int32_t    w, h, channels;
    stbi_uc*   pixels = stbi_load(absolutePath.c_str(), &w, &h, &channels, STBI_rgb_alpha);

    if (!pixels)
        return;

    m_data   = pixels;
    m_format = VK_FORMAT_R8G8B8A8_SRGB;
    m_width  = w;
    m_height = h;
}

void Texture::Destroy()
{
    m_width  = 0;
    m_height = 0;
    m_format = VK_FORMAT_UNDEFINED;

    stbi_image_free(m_data);
    m_data = nullptr;
}

const void*    Texture::GetData() const { return m_data; }
const u32      Texture::GetWidth() const { return m_width; }
const u32      Texture::GetHeight() const { return m_height; }
const VkFormat Texture::GetFormat() const { return m_format; }
} // namespace Pinut
