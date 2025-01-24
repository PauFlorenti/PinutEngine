#pragma once

namespace RED
{
enum class BufferUsage
{
    VERTEX  = 1 << 0,
    INDEX   = 1 << 1,
    UNIFORM = 1 << 2,
    STORAGE = 1 << 3,
    NONE    = 0,
};

// Enable bitwise operations for the enum class
inline BufferUsage operator|(BufferUsage lhs, BufferUsage rhs)
{
    using T = std::underlying_type_t<BufferUsage>;
    return static_cast<BufferUsage>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline BufferUsage& operator|=(BufferUsage& lhs, BufferUsage rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

inline bool operator&(BufferUsage lhs, BufferUsage rhs)
{
    using T = std::underlying_type_t<BufferUsage>;
    return (static_cast<T>(lhs) & static_cast<T>(rhs)) != 0;
}

inline bool operator!(BufferUsage lhs)
{
    using T = std::underlying_type_t<BufferUsage>;
    return !static_cast<T>(lhs);
}

struct BufferDescriptor
{
    u64         size{0};
    u64         elementSize{0};
    BufferUsage usage{BufferUsage::UNIFORM};
};
} // namespace RED
