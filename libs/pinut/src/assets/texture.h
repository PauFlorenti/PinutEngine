#pragma once

#include "asset.h"
namespace Pinut
{
class Texture final : public Asset
{
  public:
    Texture() = default;
    Texture(const u32 width, const u32 height, VkFormat format, const void* data = nullptr);
    Texture(const std::filesystem::path& filepath);
    ~Texture() = default;

    void Destroy() override;

    const u32      GetWidth() const;
    const u32      GetHeight() const;
    u8*            GetData() const;
    const VkFormat GetFormat() const;

  private:
    u32      m_width{0};
    u32      m_height{0};
    u32      m_channels{4};
    u8*      m_data{nullptr};
    VkFormat m_format{};
};
} // namespace Pinut
