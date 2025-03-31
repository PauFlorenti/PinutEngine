#pragma once

namespace RED
{
enum class TextureFormat
{
    UNDEFINED           = 0,
    R8_UNORM            = 1,
    R8_SNORM            = 2,
    R8_UINT             = 3,
    R8_SINT             = 4,
    R8_SRGB             = 5,
    R8G8_UNORM          = 6,
    R8G8_SNORM          = 7,
    R8G8_UINT           = 8,
    R8G8_SINT           = 9,
    R8G8_SRGB           = 10,
    R8G8B8_UNORM        = 11,
    R8G8B8_SNORM        = 12,
    R8G8B8_UINT         = 13,
    R8G8B8_SINT         = 14,
    R8G8B8_SRGB         = 15,
    R8G8B8A8_UNORM      = 16,
    R8G8B8A8_SNORM      = 17,
    R8G8B8A8_UINT       = 18,
    R8G8B8A8_SINT       = 19,
    R8G8B8A8_SRGB       = 20,
    R16_USCALED         = 21,
    R16_SSCALED         = 22,
    R16_UINT            = 23,
    R16_SINT            = 24,
    R16_SFLOAT          = 25,
    R16G16_UINT         = 26,
    R16G16_SINT         = 27,
    R16G16_SFLOAT       = 28,
    R16G16B16_UINT      = 29,
    R16G16B16_SINT      = 30,
    R16G16B16_SFLOAT    = 31,
    R16G16B16A16_UINT   = 32,
    R16G16B16A16_SINT   = 33,
    R16G16B16A16_SFLOAT = 34,
    R32_UINT            = 35,
    R32_SINT            = 36,
    R32_SFLOAT          = 37,
    R32G32_UINT         = 38,
    R32G32_SINT         = 39,
    R32G32_SFLOAT       = 40,
    R32G32B32_UINT      = 41,
    R32G32B32_SINT      = 42,
    R32G32B32_SFLOAT    = 43,
    R32G32B32A32_UINT   = 44,
    R32G32B32A32_SINT   = 45,
    R32G32B32A32_SFLOAT = 46,
    R64_UINT            = 47,
    R64_SINT            = 48,
    R64_SFLOAT          = 49,
    R64G64_UINT         = 50,
    R64G64_SINT         = 51,
    R64G64_SFLOAT       = 52,
    R64G64B64_UINT      = 53,
    R64G64B64_SINT      = 54,
    R64G64B64_SFLOAT    = 55,
    R64G64B64A64_UINT   = 56,
    R64G64B64A64_SINT   = 57,
    R64G64B64A64_SFLOAT = 58,
    D16_UNORM           = 59,
    D32_SFLOAT          = 60,
    S8_UINT             = 61,
    D16_UNORM_S8_UINT   = 62,
    D24_UNORM_S8_UINT   = 63,
    D32_SFLOAT_S8_UINT  = 64,
    MAX_VALUE           = 65
};
}

namespace std
{
using namespace RED;

template <>
struct hash<TextureFormat>
{
    size_t operator()(const TextureFormat& InFormat) const noexcept
    {
        return static_cast<size_t>(InFormat);
    }
};
} // namespace std
