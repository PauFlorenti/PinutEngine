#include "pch.hpp"

#include "render_device/states.h"

bool operator==(const VkVertexInputAttributeDescription& lhs,
                const VkVertexInputAttributeDescription& rhs) noexcept
{
    return lhs.location == rhs.location && lhs.binding == rhs.binding && lhs.format == rhs.format &&
           lhs.offset == rhs.offset;
}