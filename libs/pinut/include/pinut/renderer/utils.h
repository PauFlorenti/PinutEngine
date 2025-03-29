#pragma once

#include "render_device/textureFormat.h"

namespace Pinut
{
RED::TextureFormat GetFormatFromString(const std::string& s)
{
    if (s.empty())
        return RED::TextureFormat::UNDEFINED;
    else if (s == "r32g32b32a32")
        return RED::TextureFormat::R32G32B32A32_SFLOAT;
    else if (s == "d32")
        return RED::TextureFormat::D32_SFLOAT;

    return RED::TextureFormat::UNDEFINED;
}
} // namespace Pinut
