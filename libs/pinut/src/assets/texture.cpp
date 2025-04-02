#include "pch.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include <common/color.h>
#include <render_device/textureFormat.h>

#include "pinut/assets/texture.h"

namespace Pinut
{

const Texture Texture::WhiteTexture =
  Texture(1, 1, RED::TextureFormat::R8G8B8A8_UNORM, &common::WhiteRGBA, "DefaultWhiteTexture");
const Texture Texture::BlackTexture =
  Texture(1, 1, RED::TextureFormat::R8G8B8A8_UNORM, &common::BlackRGBA, "DefaultBlackTexture");
const Texture Texture::RedTexture =
  Texture(1, 1, RED::TextureFormat::R8G8B8A8_UNORM, &common::RedRGBA, "DefaultRedTexture");
const Texture Texture::GreenTexture =
  Texture(1, 1, RED::TextureFormat::R8G8B8A8_UNORM, &common::GreenRGBA, "DefaultGreenTexture");
const Texture Texture::BlueTexture =
  Texture(1, 1, RED::TextureFormat::R8G8B8A8_UNORM, &common::BlueRGBA, "DefaultBlueTexture");

Texture::Texture(const u32          width,
                 const u32          height,
                 RED::TextureFormat format,
                 const void*        data,
                 std::string        InName)
: Asset(InName, AssetType::TEXTURE),
  m_width(width),
  m_height(height),
  m_format(format)
{
    const auto dataSize = m_width * m_height * 4; // TODO This should vary depending on the format.
    m_data              = new u8[dataSize];

    if (data)
    {
        memcpy(m_data, data, dataSize);
    }
}

Texture::Texture(const std::filesystem::path& filepath)
: Asset(filepath.string(), AssetType::TEXTURE)
{
    const auto absolutePath = std::filesystem::absolute(filepath).string();
    int32_t    w, h, channels;
    stbi_uc*   pixels = stbi_load(absolutePath.c_str(), &w, &h, &channels, STBI_rgb_alpha);

    if (!pixels)
        return;

    m_data   = pixels;
    m_format = RED::TextureFormat::R8G8B8A8_SRGB;
    m_width  = w;
    m_height = h;
}

void Texture::Destroy()
{
    m_width  = 0;
    m_height = 0;
    m_format = RED::TextureFormat::UNDEFINED;

    stbi_image_free(m_data);
    m_data = nullptr;
}

u8*                      Texture::GetData() const { return m_data; }
const u32                Texture::GetWidth() const { return m_width; }
const u32                Texture::GetHeight() const { return m_height; }
const RED::TextureFormat Texture::GetFormat() const { return m_format; }
} // namespace Pinut
