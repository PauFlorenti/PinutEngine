#pragma once

namespace RED
{

inline constexpr u64 GPU_RESOURCE_INVALID = UINT64_MAX;

enum class ResourceType
{
    NONE,
    BUFFER,
    TEXTURE,
};

struct GPUResource
{
    u64          id{GPU_RESOURCE_INVALID};
    ResourceType type{ResourceType::NONE};

    bool operator==(const GPUResource& other) const
    {
        return this->id == other.id && this->type == other.type;
    }

    bool operator!=(const GPUResource& other) const { return !(*this == other); }
};

using BufferResource  = GPUResource;
using TextureResource = GPUResource;

} // namespace RED

template <>
struct std::hash<RED::GPUResource>
{
    inline size_t operator()(const RED::GPUResource& resource) const noexcept
    {
        size_t h1 = std::hash<u64>{}(resource.id);
        size_t h2 = std::hash<RED::ResourceType>{}(resource.type);
        return h1 ^ (h2 << 1);
    }
};
