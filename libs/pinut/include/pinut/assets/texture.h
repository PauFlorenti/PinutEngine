#pragma once

#include "render_device/textureFormat.h"

#include "pinut/assets/asset.h"

namespace Pinut
{
class Texture final : public Asset
{
  public:
    Texture() = delete;
    Texture(const u32          width,
            const u32          height,
            RED::TextureFormat format,
            const void*        data   = nullptr,
            std::string        InName = "");
    Texture(const std::filesystem::path& filepath);
    ~Texture() = default;

    void Destroy() override;

    const u32                GetWidth() const;
    const u32                GetHeight() const;
    u8*                      GetData() const;
    const RED::TextureFormat GetFormat() const;

  private:
    u32                m_width{0};
    u32                m_height{0};
    u32                m_channels{4};
    u8*                m_data{nullptr};
    RED::TextureFormat m_format{};

    entt::entity m_handle;
};
} // namespace Pinut
