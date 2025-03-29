#pragma once

namespace RED
{
enum class TextureFormat
{
    UNDEFINED           = 0,
    R8_UNORM            = 9,
    R8_SNORM            = 10,
    R8_UINT             = 13,
    R8_SINT             = 14,
    R8_SRGB             = 15,
    R8G8_UNORM          = 16,
    R8G8_SNORM          = 17,
    R8G8_UINT           = 20,
    R8G8_SINT           = 21,
    R8G8_SRGB           = 22,
    R8G8B8_UNORM        = 23,
    R8G8B8_SNORM        = 24,
    R8G8B8_UINT         = 27,
    R8G8B8_SINT         = 28,
    R8G8B8_SRGB         = 29,
    R8G8B8A8_UNORM      = 37,
    R8G8B8A8_SNORM      = 38,
    R8G8B8A8_UINT       = 41,
    R8G8B8A8_SINT       = 42,
    R8G8B8A8_SRGB       = 43,
    R16_USCALED         = 72,
    R16_SSCALED         = 73,
    R16_UINT            = 74,
    R16_SINT            = 75,
    R16_SFLOAT          = 76,
    R16G16_UINT         = 81,
    R16G16_SINT         = 82,
    R16G16_SFLOAT       = 83,
    R16G16B16_UINT      = 88,
    R16G16B16_SINT      = 89,
    R16G16B16_SFLOAT    = 90,
    R16G16B16A16_UINT   = 95,
    R16G16B16A16_SINT   = 96,
    R16G16B16A16_SFLOAT = 97,
    R32_UINT            = 98,
    R32_SINT            = 99,
    R32_SFLOAT          = 100,
    R32G32_UINT         = 101,
    R32G32_SINT         = 102,
    R32G32_SFLOAT       = 103,
    R32G32B32_UINT      = 104,
    R32G32B32_SINT      = 105,
    R32G32B32_SFLOAT    = 106,
    R32G32B32A32_UINT   = 107,
    R32G32B32A32_SINT   = 108,
    R32G32B32A32_SFLOAT = 109,
    R64_UINT            = 110,
    R64_SINT            = 111,
    R64_SFLOAT          = 112,
    R64G64_UINT         = 113,
    R64G64_SINT         = 114,
    R64G64_SFLOAT       = 115,
    R64G64B64_UINT      = 116,
    R64G64B64_SINT      = 117,
    R64G64B64_SFLOAT    = 118,
    R64G64B64A64_UINT   = 119,
    R64G64B64A64_SINT   = 120,
    R64G64B64A64_SFLOAT = 121,
    D16_UNORM           = 124,
    D32_SFLOAT          = 126,
    S8_UINT             = 127,
    D16_UNORM_S8_UINT   = 128,
    D24_UNORM_S8_UINT   = 129,
    D32_SFLOAT_S8_UINT  = 130,
    MAX_ENUM            = 0x7FFFFFFF
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
