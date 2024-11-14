#pragma once

namespace Pinut
{
VkFormat GetFormatFromString(const std::string& s)
{
    if (s.empty())
        return VK_FORMAT_UNDEFINED;
    else if (s == "r32g32b32a32")
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    else if (s == "d32")
        return VK_FORMAT_D32_SFLOAT;

    return VK_FORMAT_UNDEFINED;
}
} // namespace Pinut
